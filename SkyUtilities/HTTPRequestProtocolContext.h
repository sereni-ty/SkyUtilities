#pragma once

#include "RequestProtocolContext.h"

#include <curl/curl.h>
#include <string>

namespace SKU { namespace Net { namespace HTTP {

	class RequestProtocolContext : public IRequestProtocolContext
	{
		public:
			using Ptr = std::shared_ptr<RequestProtocolContext>;

		public:
			enum Method
			{
				mPOST = 0,
				mGET,
			};

		public:
			void Initialize(Method method, std::string url, std::string body);
			void Cleanup();

		protected:
			CURL *curl_handle;
			CURLcode curl_last_error;

			std::string url, body;
			Method method;

		friend class RequestManager;
	};

}}}
