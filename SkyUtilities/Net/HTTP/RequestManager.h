#pragma once

#include "Singleton.h"
#include "Events.h"

#include "Net/RequestManagerBase.h"

#include <thread>
#include <future>
#include <curl/curl.h>

#define PLUGIN_REQUEST_MANAGER_SERIALIZATION_TYPE MACRO_SWAP32('RMSU')
#define PLUGIN_REQUEST_MANAGER_SERIALIZATION_VERSION 1

#define RESPONSE_MAX_SIZE 1024 * 128 // TODO: Configuration

namespace SKU::Net::HTTP { // TODO: (protection) request creation frequency

	class RequestManager : public Singleton<RequestManager>, public SKU::Net::RequestManagerBase, public IEventHandler
	{
		IS_SINGLETON_CLASS(RequestManager)

		private:
			Request::Ptr GetRequestByHandle(CURL *curl_handle);

		public:
			void Initialize();

			void Stop();
			void Start();

		public:
			static void Process();
			
		private:
			bool SetupRequests();
			bool PerformRequests();
			bool CheckPendingRequests();
		
		public:
			virtual void OnRequestAdded(Request::Ptr request) override;
			virtual void OnRequestRemoval(Request::Ptr request) override;

		public:
			static size_t OnRequestResponse(char* data, size_t size, size_t nmemb, void *request_id);

		public:
			void OnSKSESaveGame(SKSESerializationInterface *serilization_interface);
			void OnSKSELoadGame(SKSESerializationInterface *serilization_interface, SInt32 type, SInt32 version, SInt32 length);

		private:
			bool should_run;
			std::future<void> processing_thread;

			CURLM *curl_handle;
			CURLMcode curl_last_error;
	};

}
