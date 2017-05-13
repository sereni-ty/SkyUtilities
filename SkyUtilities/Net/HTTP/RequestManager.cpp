#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"

#include "Net/NetInterface.h"

#include "Plugin.h"
#include "PapyrusEventHandler.h"

#include <mutex>

namespace SKU::Net::HTTP {

	RequestManager::RequestManager() 
		: should_run(false), curl_handle(nullptr)
	{

	}

	RequestManager::~RequestManager() 
	{
		Stop();
	}

	void RequestManager::Stop()
	{
		should_run = false;

		if (pool.Get().empty() != true)
		{
			for (auto request : pool.Get())
			{
				if (request == nullptr)
					continue;

				request->Lock();

				Request::State state = request->GetState();

				if (state != Request::sOK && state != Request::sFailed) // Manager stops. Remaining, unprocessed requests default to failure state
				{
					request->SetState(Request::sFailed);
				}

				RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

				if (ctx == nullptr)
					continue;

				Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Request (id: %d) is being removed (curl_multi_remove_handle()).", request->GetID());

				if (curl_handle != nullptr)
				{
					curl_last_error = curl_multi_remove_handle(curl_handle, ctx->curl_handle);

					if (curl_last_error != CURLM_OK)
					{
						Plugin::Log(LOGL_INFO, "(HTTP) RequestManager: curl_multi_remove_handle() failed with error code %d. This shouldn't really happen.", curl_last_error);
					}
				}

				request->Stop();
			}
		}

		if (curl_handle != nullptr)
		{
			curl_multi_cleanup(curl_handle);
			curl_handle = nullptr;
		}

		Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Stopped.");
	}

	void RequestManager::Start()
	{
		static std::mutex thread_creation_mtx;
		std::lock_guard<std::mutex> guard(thread_creation_mtx);

		Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Start");

		should_run = true;

		if (curl_handle == nullptr)
			curl_handle = curl_multi_init();

		if (processing_thread.valid() == false 
		|| processing_thread.wait_for(std::chrono::seconds(0)) != std::future_status::timeout)
		{
			Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Starting Thread (again)");
			processing_thread = std::move(std::async(RequestManager::Process));
		}
	}

	void RequestManager::RemoveRequest(Request::Ptr request)
	{
		if (request == nullptr)
		{
			return;
		}

		request->Lock();

		curl_multi_remove_handle(curl_handle, request->GetProtocolContext<RequestProtocolContext>()->curl_handle);

		request->Stop();

		Request::State state = request->GetState();

		if (state != Request::sOK && state != Request::sFailed)
		{
			request->SetState(Request::sFailed); // TODO: (Consideration) Necessary to implement a separate state for stopped requests?
		}

		request->Unlock();
	}

	void RequestManager::Process()
	{
		RequestManager *mgr = RequestManager::GetInstance();
		std::list<Request::Ptr> requests;
		int handle_reset_counter = 0, no_request_counter = 0;
		
		mgr->curl_last_error = CURLM_OK;		

		Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Starting to process requests");

		while (Plugin::GetInstance()->IsGameReady()
		&& mgr->should_run 
		&& no_request_counter < 3)
		{			
			try
			{
				/* check: on pending requests */
				bool processed_pending_requests = mgr->CheckPendingRequests();

				if (mgr->pool.GetCountByStateExceptions({ Request::sFailed, Request::sOK }) == 0	// Only work if there are requests left to work on
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
				(void)mgr->PerformRequests();
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
						Net::Interface::GetInstance()->Stop();
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
							if (request != nullptr)
								request->SetState(Request::sReady);

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
			request->Lock();
			RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

			Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Setting up request (id: %d)", request->GetID());

			curl_easy_setopt(ctx->curl_handle, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);
			curl_easy_setopt(ctx->curl_handle, CURLOPT_URL, ctx->url.c_str());
			curl_easy_setopt(ctx->curl_handle, CURLOPT_WRITEFUNCTION, OnRequestResponse);
			curl_easy_setopt(ctx->curl_handle, CURLOPT_WRITEDATA, (void*) request->GetID());
			curl_easy_setopt(ctx->curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
			curl_easy_setopt(ctx->curl_handle, CURLOPT_TIMEOUT_MS, request->GetTimeout());

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

		return true;
	}

	bool RequestManager::PerformRequests()
	{
		int active_requests = 0;
		auto requests = std::move(pool.GetRequestsByState(Request::sReady));

		Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: %d requests ready to be added to the multi handle", requests.size());

		for (Request::Ptr &request : requests)
		{
			request->Lock();

			curl_last_error = curl_multi_add_handle(curl_handle, request->GetProtocolContext<RequestProtocolContext>()->curl_handle);

			if (curl_last_error != CURLM_OK 
			&& curl_last_error != CURLM_ADDED_ALREADY)
			{
				request->Unlock();

				switch (curl_last_error)
				{
					case CURLM_BAD_EASY_HANDLE: // Could be a request removed? Anyway, try to clean that one up. Your journey is over here, little one.
					{
						Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: Request (id: %d) has bad handle", request->GetID());

						request->Stop();
						request->SetState(Request::sFailed);
					} break;
					
					default: throw std::exception();
				}
			}
			else
			{
				request->SetState(Request::sPending);
				request->Unlock();
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
					goto call_again;
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

		auto fnGetRequestByHandle = [&](CURL* curl_handle, std::list<Request::Ptr> &requests) -> Request::Ptr {
			Request::Ptr found_request;

			for (Request::Ptr request : requests)
			{
				if (request->GetProtocolContext<RequestProtocolContext>()->curl_handle == curl_handle)
				{
					found_request = request;
				}
			}

			return found_request;
		};

		/* get pending requests, there's no need to iterate through the whole set */
		auto requests = std::move(pool.GetRequestsByState(Request::sPending));

		Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestManager: %d requests being processed", requests.size());

		while ((info = curl_multi_info_read(curl_handle, &message_count))/* && message_count > 0*/)	// message_count seems to be irrelevant since it'll return 
		{																							// nullptr anyway if there is nothing to do anymore	
			handled_requests++;

			if (info->easy_handle == nullptr) // Huh?
			{
				Plugin::Log(LOGL_INFO, "(HTTP) RequestManager: Invalid CURL handle in CURL response message. Internal error?");
				continue;
			}
			
			Request::Ptr &request = fnGetRequestByHandle(info->easy_handle, requests);

			if (request == nullptr) // Request not found. 
			{						// TODO: Request state was changed? Check that.
				Plugin::Log(LOGL_WARNING, "(HTTP) RequestManager: Request was answered but (internally) not found.");

				curl_easy_cleanup(info->easy_handle); // Let the dead rest.				
				continue;
			}

			request->Lock();

			if (request->GetState() <= Request::sOK)	// Was cleaned up before but curl apparently had
			{											// several messages in its queue for this request.
				request->Unlock();
				continue;
			}

			RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();

			request->SetState(info->data.result == CURLE_OK ? Request::sOK : Request::sFailed);
			ctx->curl_last_error = info->data.result;

			curl_last_error = curl_multi_remove_handle(curl_handle, ctx->curl_handle);

			Plugin::Log(LOGL_DETAILED, "Request (id: %d, error code: %d) has %s", 
				request->GetID(), 
				info->data.result,
				info->data.result == CURLE_OK ? "finished successfully" : "failed");

			PapyrusEvent::Args &args = PapyrusEvent::Args { std::make_any<int>(request->GetID()), std::make_any<bool>(request->GetState() == Request::sFailed) };

			if (info->data.result == CURLE_OK)
			{
				long response_code;
				curl_easy_getinfo(ctx->curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

				args.emplace_back(std::make_any<int>(response_code));
				args.emplace_back(std::make_any<std::string>(ctx->response));
			}
			else
			{
				args.emplace_back(std::make_any<int>(-1));
				args.emplace_back(std::make_any<std::string>());
			}

			PapyrusEventHandler::GetInstance()->Send(Interface::GetEventString(Interface::evHTTPRequestFinished), std::move(args));

			request->Stop();
			request->Unlock();

			if (curl_last_error != CURLM_OK)
			{
				throw std::exception();
			}
		}

		return handled_requests > 0;
	}

	size_t RequestManager::OnRequestResponse(char* data, size_t size, size_t nmemb, void *request_id)
	{
		Request::Ptr &request = RequestManager::GetInstance()->pool.GetRequestByID((int)request_id);

		if (request == nullptr)
			return -1;

		Plugin::Log(LOGL_VERBOSE, "RequestManager: Request (id: %d) was answered with %d bytes.", (int)request_id, size*nmemb);

		request->GetProtocolContext<RequestProtocolContext>()->response.append(data, size*nmemb);

		return size*nmemb;
	}

}
