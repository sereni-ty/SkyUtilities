#pragma once

#include "Singleton.h"
#include "Serialization.h"

#include "Net/RequestManagerBase.h"

#include <thread>
#include <future>
#include <curl/curl.h>

#define PLUGIN_REQUEST_MANAGER_SERIALIZATION_TYPE 'RMSU'
#define PLUGIN_REQUEST_MANAGER_SERIALIZATION_VERSION 1

namespace SKU::Net::HTTP {

	class RequestManager : public Singleton<RequestManager>, public SKU::Net::RequestManagerBase, public ISerializeable
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

		public:
			void Save(SKSESerializationInterface *serilization_interface);
			void Load(SKSESerializationInterface *serilization_interface);

		private:
			bool should_run;
			std::future<void> processing_thread;

			CURLM *curl_handle;
			CURLMcode curl_last_error;
	};

}
