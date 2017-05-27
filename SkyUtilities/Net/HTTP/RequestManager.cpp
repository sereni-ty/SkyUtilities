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
		if(curl_handle == nullptr)
		{
			curl_handle = curl_multi_init();
		}		
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

		Initialize();

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

		while ((info = curl_multi_info_read(curl_handle, &message_count))/* && message_count > 0*/)	// message_count seems to be irrelevant since it'll return 
		{																							// nullptr anyway if there is nothing to do anymore	
			handled_requests++;

			if (info->easy_handle == nullptr) // Huh?
			{
				Plugin::Log(LOGL_INFO, "(HTTP) RequestManager: Invalid CURL handle in CURL response message. Internal error?");
				continue;
			}
			
			Request::Ptr &request = GetRequestByHandle(info->easy_handle);

			if (request == nullptr) // Request not found. 
			{						// TODO: Request state was changed? Check that.
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

			request->SetState(info->data.result == CURLE_OK ? Request::sOK : Request::sFailed);
			ctx->curl_last_error = info->data.result; // TODO: Think about how to pass info data..

			curl_last_error = curl_multi_remove_handle(curl_handle, ctx->curl_handle);

			Plugin::Log(LOGL_DETAILED, "Request (id: %d, error code: %d) has %s",
				request->GetID(),
				info->data.result,
				info->data.result == CURLE_OK ? "finished successfully" : "failed");

			if (request->GetHandler() == nullptr)
			{
				Plugin::Log(LOGL_VERBOSE, "RequestManager: Request (id: %d) has no handler to call. Skipping.",
					request->GetID());
			}
			else
			{
				Plugin::Log(LOGL_VERBOSE, "RequestManager: Calling request (id: %d) handler.",
					request->GetID());

				request->GetHandler()->OnRequestFinished(request);
			}
			
			request->Stop();
			request->Unlock();

			if (curl_last_error != CURLM_OK)
			{
				throw std::exception();
			}
		}

		return handled_requests > 0;
	}

	void RequestManager::OnRequestAdded(Request::Ptr request)
	{
		if (request == nullptr)
		{
			return;
		}

		Initialize();

		if (request->GetState() != Request::sFailed)
		{
			curl_multi_add_handle(curl_handle, request->GetProtocolContext<RequestProtocolContext>()->curl_handle);
		}

		Start();
	}

	void RequestManager::OnRequestRemoval(Request::Ptr request)
	{
		if (request == nullptr || curl_handle == nullptr)
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

	size_t RequestManager::OnRequestResponse(char* data, size_t size, size_t nmemb, void *request_id)
	{
		Request::Ptr &request = RequestManager::GetInstance()->pool.GetRequestByID((int)request_id);

		if (request == nullptr)
		{
			return -1;
		}

		RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();
		size_t new_size = nmemb*size + ctx->response.size();

		if (new_size > RESPONSE_MAX_SIZE)
		{
			Plugin::Log(LOGL_VERBOSE, "RequestManager: Request (id: %d) exceeded response limit (%dKB). Stopping request.", 
				request->GetID(), RESPONSE_MAX_SIZE / 1024);

			request->SetState(Request::sFailed);
			//request->Stop(); 

			return -1;
		}

		Plugin::Log(LOGL_VERBOSE, "RequestManager: Request (id: %d) was answered with %d bytes.", (int)request_id, size*nmemb);

		request->GetProtocolContext<RequestProtocolContext>()->response.append(data, size*nmemb);

		return size*nmemb;
	}

	void RequestManager::OnSKSESaveGame(SKSESerializationInterface *serilization_interface)
	{
		Plugin::Log(LOGL_VERBOSE, "RequestManager: Saving unprocessed requests.");

		uint32_t unprocessed = 0;
		bool write_fail = true;
		
		if ((unprocessed = pool.GetCountByStateExceptions({ Request::sFailed, Request::sOK })) == 0)
		{
			Plugin::Log(LOGL_VERBOSE, "RequestManager: Nothing to save.");
			return;
		}

		if (serilization_interface->OpenRecord(PLUGIN_REQUEST_MANAGER_SERIALIZATION_TYPE, PLUGIN_REQUEST_MANAGER_SERIALIZATION_VERSION) == false)
		{
			Plugin::Log(LOGL_WARNING, "RequestManager: Unable to save data.");
			return;
		}

		Plugin::Log(LOGL_VERBOSE, "RequestManager: %d unprocessed requests going to be saved", unprocessed);

		if (serilization_interface->WriteRecordData(&unprocessed, 4) == true)
		{
			for (Request::Ptr request : pool.Get())
			{
				if (request->GetState() == Request::sFailed
				|| request->GetState() == Request::sOK)
				{
					continue;
				}

				Plugin::Log(LOGL_VERBOSE, "RequestManager: Saving request. %d requests to save remaining.", --unprocessed);

				uint32_t tmp;
				RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();
				write_fail = true;

				if (ctx == nullptr)
				{
					Plugin::Log(LOGL_VERBOSE, "RequestManager: Request has no context information. Skipping.");
					write_fail = false;
					continue;
				}

				FAIL_BREAK_WRITE(serilization_interface, &(tmp = request->GetID()), 4);
				FAIL_BREAK_WRITE(serilization_interface, &(tmp = request->GetTimeout()), 4);

				FAIL_BREAK_WRITE(serilization_interface, &(tmp = ctx->url.size()), 4);
				FAIL_BREAK_WRITE(serilization_interface, ctx->url.c_str(), tmp);

				FAIL_BREAK_WRITE(serilization_interface, &(tmp = ctx->body.size()), 4);
				FAIL_BREAK_WRITE(serilization_interface, ctx->body.c_str(), tmp);

				FAIL_BREAK_WRITE(serilization_interface, &ctx->method, sizeof(RequestProtocolContext::Method));

				write_fail = false;
			}
		}

		if (write_fail == true)
		{
			Plugin::Log(LOGL_WARNING, "RequestManager: Failed to write save data");
		}
	}

	void RequestManager::OnSKSELoadGame(SKSESerializationInterface *serilization_interface, SInt32 type, SInt32 version, SInt32 length)
	{
		if (type != PLUGIN_REQUEST_MANAGER_SERIALIZATION_TYPE)
		{
			return;
		}

		if (length == 0)
		{
			Plugin::Log(LOGL_INFO, "RequestManager: Nothing to load.");
			return;
		}

		if (version != PLUGIN_REQUEST_MANAGER_SERIALIZATION_VERSION)
		{
			Plugin::Log(LOGL_WARNING, "RequestManager: Unsupported data version.");
			return;
		}

		Plugin::Log(LOGL_VERBOSE, "RequestManager: Loading unprocessed requests.");

		uint32_t unprocessed = 0;
		bool read_fail = false;

		if (serilization_interface->ReadRecordData(&unprocessed, 4) != 4)
		{
			Plugin::Log(LOGL_WARNING, "RequestManager: Unable to load data from save.");
			return;
		}
		
		Plugin::Log(LOGL_VERBOSE, "RequestManager: Loading %d requests from save.", unprocessed);

		while(unprocessed-- > 0)
		{
			Request::Ptr request = nullptr;
			HTTP::RequestProtocolContext::Ptr ctx = nullptr;

			size_t tmp;
			std::vector<char> buf;

			read_fail = true;

			FAIL_BREAK_READ(serilization_interface, &tmp, sizeof(int)); // id

			request = Request::Create<HTTP::RequestProtocolContext>(tmp);
			ctx = request->GetProtocolContext<HTTP::RequestProtocolContext>();

			FAIL_BREAK_READ(serilization_interface, &tmp, sizeof(size_t)); // timeout
			request->SetTimeout(tmp);

			FAIL_BREAK_READ(serilization_interface, &tmp, sizeof(size_t)); // url size

			try 
			{
				buf.resize(tmp + 1, 0);
			}
			catch (std::exception)
			{
				break;
			}

			FAIL_BREAK_READ(serilization_interface, &buf[0], tmp); // url
			ctx->url = std::string(&buf[0]);

			FAIL_BREAK_READ(serilization_interface, &tmp, sizeof(size_t)); // body size

			try
			{
				buf.clear();
				buf.resize(tmp + 1, 0);
			}
			catch (std::exception)
			{
				break;
			}

			FAIL_BREAK_READ(serilization_interface, &buf[0], tmp); // body
			ctx->body = std::string(&buf[0]);

			FAIL_BREAK_READ(serilization_interface, &ctx->method, sizeof(HTTP::RequestProtocolContext::Method));

			ctx->Initialize();
			AddRequest(request);

			read_fail = false;
		}

		if (read_fail == true)
		{
			Plugin::Log(LOGL_WARNING, "RequestManager: Unable to load data from save");
			return;
		}
	}

}
