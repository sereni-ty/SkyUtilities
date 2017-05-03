#pragma once

#include "Singleton.h"
#include "RequestManagerBase.h"

#include <thread>
#include <future>
#include <curl/curl.h>

namespace SKU { namespace Net { namespace HTTP {

	class RequestManager : public Singleton<RequestManager>, public SKU::Net::RequestManagerBase
	{
		IS_SINGLETON_CLASS(RequestManager)
			
		public:
			void Stop();
			void Start();
			
		public:
			void RemoveRequest(Request::Ptr request);

		public:
			static void Process();
			
		private:
			bool SetupRequests();
			bool PerformRequests();
			bool CheckPendingRequests();

		public:
			static size_t OnRequestResponse(char* data, size_t size, size_t nmemb, void *request_id);

		private:
			bool should_run;
			std::future<void> processing_thread;

			CURLM *curl_handle;
			CURLMcode curl_last_error;
	};

}}}
