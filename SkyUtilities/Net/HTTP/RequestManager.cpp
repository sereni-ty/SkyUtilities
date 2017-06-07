#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"
#include "Net/HTTP/BasicRequestEventHandler.h"
#include "Net/HTTP/ModInfoRequestEventHandler.h"

#include "Net/NetInterface.h"

#include "Plugin.h"
#include "PapyrusEventManager.h"

#include <mutex>

namespace SKU::Net::HTTP {
  RequestManager::RequestManager()
    : should_run(false), curl_handle(nullptr)
  {
    Initialize();
  }

  RequestManager::~RequestManager()
  {
    Cleanup();
  }

  inline Request::Ptr RequestManager::GetRequestByHandle(CURL *curl_handle)
  {
    for (Request::Ptr request : pool.Get())
    {
      if (request->GetProtocolContext<RequestProtocolContext>()->curl_handle == curl_handle)
      {
        return request;
      }
    }

    return nullptr;
  }

  void RequestManager::Initialize()
  {
    should_run = false;
    processing_thread = std::future<void>();

    if (curl_handle == nullptr)
    {
      curl_handle = curl_multi_init();
    }
  }

  void RequestManager::Cleanup()
  {
    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Cleaning up..");

    while (pool.Get().empty() == false)
    {
      Request::Ptr request = *pool.Get().begin();

      RemoveRequest(request);
    }

    if (curl_handle != nullptr)
    {
      curl_multi_cleanup(curl_handle);
      curl_handle = nullptr;
    }

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Cleaned up..");
  }

  void RequestManager::Stop()
  {
    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Stopping..");

    should_run = false;

    if (processing_thread.valid() == true)
    {
      processing_thread.wait();
    }

    processing_thread = std::future<void>();

    if (curl_handle == nullptr && pool.Get().empty() == true)
    {
      return; // Already stopped.
    }

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Stopping requests..");
    for (auto request : pool.Get())
    {
      if (request == nullptr)
      {
        continue;
      }

      request->Lock();
      RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

      if (ctx == nullptr)
      {
        Plugin::Log(LOGL_DETAILED, "(HTTP) RequestManager: Request (id: %d) has no context.", request->GetID());

        request->Unlock();
        continue;
      }

      if (ctx->curl_handle == nullptr)
      {
        Plugin::Log(LOGL_DETAILED, "(HTTP) RequestManager: Request (id: %d) has invalid handle.", request->GetID());

        request->Unlock();
        continue;
      }

      if (request->GetState() > Request::sOK)
      {
        CURLcode res = curl_easy_pause(ctx->curl_handle, CURLPAUSE_ALL);

        if (res != CURLE_OK)
        {
          Plugin::Log(LOGL_DETAILED, "(HTTP) RequestManager: Failed to pause request (id: %d, error: %d).", request->GetID(), res);
        }
        else
        {
          Plugin::Log(LOGL_DETAILED, "(HTTP) RequestManager: Paused request (id: %d).", request->GetID());

          request->Stop();
        }
      }

      request->Unlock();
    }

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Stopped.");
  }

  void RequestManager::Start() // TODO: Resume paused requests
  {
    static std::mutex thread_creation_mtx;
    std::lock_guard<std::mutex> guard(thread_creation_mtx);

    if (curl_handle == nullptr)
    {
      Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Not starting due to missing initialization.");
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Start");

    should_run = true;

    if (processing_thread.valid() == false
      || processing_thread.wait_for(std::chrono::seconds(0)) != std::future_status::timeout)
    {
      Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Starting Thread (again)");
      processing_thread = std::move(std::async(RequestManager::Process));
    }
  }

  void RequestManager::Process()
  {
    RequestManager::Ptr &mgr = Plugin::GetInstance()->GetNetInterface()->GetHTTPRequestManager();
    int handle_reset_counter = 0, no_request_counter = 0;

    mgr->curl_last_error = CURLM_OK;

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Starting to process requests");

    while (Plugin::GetInstance()->IsActive()
      && mgr->should_run
      && no_request_counter < 3)
    {
      try
      {
        /* check: on pending requests */
        bool processed_pending_requests = mgr->CheckPendingRequests();

        if (mgr->pool.GetCountByStateExceptions({Request::sFailed, Request::sOK, Request::sBlacklisted}) == 0	// Only work if there are requests left to work on
          && processed_pending_requests == false)												// Try again in 200ms.
        {
          std::this_thread::sleep_for(std::chrono::milliseconds(200));
          no_request_counter++;
          continue;
        }

        no_request_counter = 0;

        /* check: requests which weren't yet initialized (curl_setopt) */
        mgr->SetupRequests();

        /* check: requests which were initialized but not yet added to the multi handle */
        (void) mgr->PerformRequests();
      }
      catch (std::exception)
      {
        switch (mgr->curl_last_error)
        {
          case CURLM_OK: break;

          case CURLM_OUT_OF_MEMORY:
          {
            Plugin::Log(LOGL_CRITICAL, "(HTTP) RequestManager: CURL ran OOM. Stopping Plugin.");
            Plugin::GetInstance()->Stop();
            continue;
          } break;

          case CURLM_INTERNAL_ERROR:
          {
            // Note: Try to reset CURL (Easy, Multi. Global might be tricky). Does it make sense, though? All threads (additional in the future?)
            //       would need to wait for the reset to be done.
            // Follow Up: Keep track of reset count: Did it solve the problem?
            //

            Plugin::Log(LOGL_CRITICAL, "(HTTP) RequestManager: Internal CURL error occured. Stopping Net Interface.");
            Plugin::GetInstance()->GetNetInterface()->Stop();
            continue;
          } break;

          case CURLM_BAD_HANDLE: // Try to reset handle
          {
            if (mgr->should_run == false
              && mgr->curl_handle == nullptr) // Stop was called.
            {
              return;
            }

            Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Resetting bad curl handle.");

            for (Request::Ptr& request : mgr->pool.GetRequestsByState(Request::sPending))
            {
              if (request != nullptr)
              {
                request->SetState(Request::sReady);
              }
            }

            mgr->curl_handle = curl_multi_init();

            if (mgr->curl_handle != nullptr)
              continue;
          }

          default:
          {
            Plugin::Log(LOGL_WARNING, "(HTTP) RequestManager: Unhandled CURLM error: %d. Stopping Request Manager.", mgr->curl_last_error);
            mgr->Stop();
            continue;
          }
        }
      }

      std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Request processing thread finished its work or was interrupted (error code: %d)", mgr->curl_last_error);
  }

  bool RequestManager::SetupRequests()
  {
    auto requests = std::move(pool.GetRequestsByState(Request::sWaitingForSetup));

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: %d requests waiting for setup", requests.size());

    for (Request::Ptr &request : requests)
    {
      RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

      if (ctx == nullptr)
      {
        RemoveRequest(request);
      }
      else if (request->GetState() == Request::sWaitingForSetup)
      {
        request->Lock();

        Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Setting up request (id: %d)", request->GetID());

        curl_easy_setopt(ctx->curl_handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
        curl_easy_setopt(ctx->curl_handle, CURLOPT_URL, ctx->url.c_str());
        curl_easy_setopt(ctx->curl_handle, CURLOPT_WRITEFUNCTION, OnRequestResponse);
        curl_easy_setopt(ctx->curl_handle, CURLOPT_WRITEDATA, (void*) request->GetID());
        curl_easy_setopt(ctx->curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
        curl_easy_setopt(ctx->curl_handle, CURLOPT_TIMEOUT_MS, request->GetTimeout());
        curl_easy_setopt(ctx->curl_handle, CURLOPT_FOLLOWLOCATION, TRUE);

        switch (ctx->method)
        {
          case RequestProtocolContext::mPOST:
          {
            curl_easy_setopt(ctx->curl_handle, CURLOPT_COPYPOSTFIELDS, ctx->body.c_str());
          } break;
        }

        request->SetState(Request::sReady);
        request->Unlock();
      }
    }

    return true;
  }

  bool RequestManager::PerformRequests()
  {
    int active_requests = 0;
    auto requests = std::move(pool.GetRequestsByState(Request::sReady));

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: %d requests ready to be added to the multi handle", requests.size());

    for (Request::Ptr &request : requests)
    {
      if (request->GetState() == Request::sReady)
      {
        RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

        if (ctx == nullptr)
        {
          RemoveRequest(request);
        }

        else
        {
          request->Lock();

          curl_last_error = curl_multi_add_handle(curl_handle, ctx->curl_handle);

          switch (curl_last_error)
          {
            case CURLM_ADDED_ALREADY:
            case CURLM_OK:
              break;

            case CURLM_BAD_EASY_HANDLE: // Could be a request removed? Anyway, try to clean that one up. Your journey is over here, little one.
            {
              Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Request (id: %d) has bad handle", request->GetID());

              request->SetState(Request::sFailed);
              request->Unlock();

              RemoveRequest(request);
            } continue;

            default:
            {
              request->Unlock();
              throw std::exception();
            } break;
          }

          request->SetState(Request::sPending);
          request->Unlock();
        }
      }
    }

    /* perform */
    int call_counter = 0;

  call_again:
    curl_last_error = curl_multi_perform(curl_handle, &active_requests);
    call_counter++;

    switch (curl_last_error)
    {
      case CURLM_OK: break;

      case CURLM_CALL_MULTI_PERFORM:
      {
        if (call_counter < 3)
        {
          goto call_again;
        }
      } break;

      default: throw std::exception();
    }

    return active_requests > 0;
  }

  bool RequestManager::CheckPendingRequests()
  {
    int handled_requests = 0;
    int message_count;
    CURLMsg *info;

    while ((info = curl_multi_info_read(curl_handle, &message_count))/* && message_count > 0*/)	// message_count seems to be irrelevant since it'll return
    {																							// nullptr anyway if there is nothing to do anymore
      handled_requests++;

      if (info->easy_handle == nullptr) // Huh?
      {
        Plugin::Log(LOGL_INFO, "(HTTP) RequestManager: Invalid CURL handle in CURL response message. Internal error?");
        continue;
      }

      Request::Ptr request = GetRequestByHandle(info->easy_handle);

      if (request == nullptr) // Request not found.
      {
        Plugin::Log(LOGL_WARNING, "(HTTP) RequestManager: Request was answered but (internally) not found.");

        curl_easy_cleanup(info->easy_handle); // Let the dead rest.
        continue;
      }

      request->Lock();

      if (request->GetState() <= Request::sOK)	// Was cleaned up / stopped before but curl apparently had
      {											// several messages in its queue for this request.
        request->Unlock();
        continue;
      }

      RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

      if (ctx == nullptr)
      {
        Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Request (id: %d) without context.", request->GetID());

        request->Unlock();
        continue;
      }

      request->SetState(info->data.result == CURLE_OK ? Request::sOK : Request::sFailed);
      ctx->curl_last_error = info->data.result;

      Plugin::Log(LOGL_DETAILED, "Request (id: %d, error code: %d) has %s",
        request->GetID(),
        info->data.result,
        info->data.result == CURLE_OK ? "finished successfully" : "failed");

      if (request->GetHandler() == nullptr)
      {
        Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Request (id: %d) has no handler to call. Skipping.",
          request->GetID());
      }
      else
      {
        Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Calling request (id: %d) handler.",
          request->GetID());

        request->GetHandler()->OnRequestFinished(request);
      }

      request->Unlock();

      RemoveRequest(request);
    }

    return handled_requests > 0;
  }

  void RequestManager::OnRequestAdded(Request::Ptr request)
  {
    if (Plugin::GetInstance()->IsActive() == true)
    {
      Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Starting..");
      Start();
    }
  }

  void RequestManager::OnRequestRemoval(Request::Ptr request)
  {
    if (request == nullptr)
    {
      return;
    }

    request->Lock();

    RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

    if (ctx != nullptr && curl_handle != nullptr)
    {
      curl_multi_remove_handle(curl_handle, ctx->curl_handle);
    }

    if (request->GetState() > Request::sOK)
    {
      request->SetState(Request::sFailed);
    }

    request->Cleanup();
    request->Unlock();
  }

  size_t RequestManager::OnRequestResponse(char* data, size_t size, size_t nmemb, void *request_id)
  {
    Request::Ptr &request = Plugin::GetInstance()->GetNetInterface()->GetHTTPRequestManager()->pool.GetRequestByID((int) request_id);

    if (request == nullptr)
    {
      return -1;
    }

    RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();
    size_t new_size = nmemb*size + ctx->response.size();

    if (new_size > RESPONSE_MAX_SIZE)
    {
      Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Request (id: %d) exceeded response limit (%dKB). Stopping request.",
        request->GetID(), RESPONSE_MAX_SIZE / 1024);

      request->SetState(Request::sFailed);
      return -1; // Will appear as info message in CheckPendingRequests()
    }

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Request (id: %d) was answered with %d bytes.", (int) request_id, size*nmemb);
    request->GetProtocolContext<RequestProtocolContext>()->response.append(data, size*nmemb);

    return size*nmemb;
  }

  void RequestManager::Serialize(std::stack<ISerializeable::SerializationEntity> &serialized_entities)
  {
    SerializationEntity serialized;
    uint32_t unfinished_requests;

    std::get<ISerializeable::idType>(serialized) = PLUGIN_REQUEST_MANAGER_SERIALIZATION_TYPE;
    std::get<ISerializeable::idVersion>(serialized) = PLUGIN_REQUEST_MANAGER_SERIALIZATION_VERSION;
    std::get<ISerializeable::idStream>(serialized) = std::stringstream();

    if ((unfinished_requests = pool.GetCountByStateExceptions({Request::sBlacklisted, Request::sFailed, Request::sOK})) == 0)
    {
      Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Nothing to save.");
      return;
    }

    SerializeIntegral(serialized, unfinished_requests);

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Saving %d requests.",
      unfinished_requests);

    for (Request::Ptr request : pool.Get())
    {
      RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

      if (request->GetState() <= Request::sOK || ctx == nullptr)
      {
        continue;
      }

      SerializeIntegral(serialized, request->GetID());
      SerializeIntegral(serialized, request->GetTimeout());
      SerializeIntegral(serialized, request->GetHandler()->GetTypeID());
      SerializeString(serialized, ctx->url);
      SerializeString(serialized, ctx->body);
      SerializeIntegral<uint32_t>(serialized, ctx->method);

      if (std::get<ISerializeable::idStream>(serialized).fail() == true)
      {
        break;
      }

      unfinished_requests--;
    }

    if (unfinished_requests != 0) // TODO: handle that
    {
      Plugin::Log(LOGL_WARNING, "(HTTP) RequestManager: Reverting due to unmatched request count.");
      // for now: revert.
      std::get<ISerializeable::idStream>(serialized).str("");
    }

    serialized_entities.push(std::move(serialized));
  }

  void RequestManager::Deserialize(ISerializeable::SerializationEntity &serialized)
  {
    int unfinished_requests = 0;

    int id;
    uint32_t timeout, handler_type_id, method;
    std::string url, body;

    if (IsRequestedSerialization(serialized) == false)
    {
      return;
    }

    if (std::get<ISerializeable::idStream>(serialized).tellp() == std::streampos(0))
    {
      Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Nothing to load.");
      return;
    }

    DeserializeIntegral(serialized, unfinished_requests);

    Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Loading %d requests from save.",
      unfinished_requests);

    for (int i = 0; i < unfinished_requests; i++)
    {
      Plugin::Log(LOGL_DETAILED, "(HTTP) RequestManager: Deserializing Request..");

      id = timeout = handler_type_id = method = 0;
      url.clear();
      body.clear();

      DeserializeIntegral(serialized, id);
      DeserializeIntegral(serialized, timeout);
      DeserializeIntegral(serialized, handler_type_id);
      DeserializeString(serialized, url);
      DeserializeString(serialized, body);
      DeserializeIntegral(serialized, method);

      if (std::get<ISerializeable::idStream>(serialized).fail() == true)
      {
        Plugin::Log(LOGL_WARNING, "(HTTP) RequestManager: Failed to deserialize data.. Aborting.");
        return;
      }

      Plugin::Log(LOGL_DETAILED, "(HTTP) RequestManager: Deserialized.. ID=%d, Timeout=%d, Handler=%d, URL=%s, Body=%d (length), Method=%d",
        id, timeout, handler_type_id, url.c_str(), body.length(), method);

      Request::Ptr request = Request::Create<RequestProtocolContext>(id);
      HTTP::RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

      switch (handler_type_id)
      {
        case HTTP::BasicRequestEventHandler::TypeID:
        {
          request->SetHandler(std::make_shared<HTTP::BasicRequestEventHandler>());
        } break;

        case HTTP::NexusModInfoRequestEventHandler::TypeID:
        {
          request->SetHandler(std::make_shared<HTTP::NexusModInfoRequestEventHandler>());
        } break;

        case HTTP::LLabModInfoRequestEventHandler::TypeID:
        {
          request->SetHandler(std::make_shared<HTTP::LLabModInfoRequestEventHandler>());
        } break;

        default: continue;
      }

      request->SetTimeout(timeout);
      ctx->Initialize(static_cast<HTTP::RequestProtocolContext::Method>(method), url, body);
      AddRequest(std::move(request));
    }

    if (std::get<ISerializeable::idStream>(serialized).fail() == true)
    {
      Plugin::Log(LOGL_WARNING, "(HTTP) RequestManager: Error occurred while loading.");
    }
  }

  bool RequestManager::IsRequestedSerialization(ISerializeable::SerializationEntity &serialized)
  {
    if (std::get<ISerializeable::idType>(serialized) != PLUGIN_REQUEST_MANAGER_SERIALIZATION_TYPE)
    {
      return false;
    }

    if (std::get<ISerializeable::idVersion>(serialized) != PLUGIN_REQUEST_MANAGER_SERIALIZATION_VERSION)
    {
      Plugin::Log(LOGL_WARNING, "(HTTP) RequestManager: Unsupported data version.");
      return false;
    }

    return true;
  }
}