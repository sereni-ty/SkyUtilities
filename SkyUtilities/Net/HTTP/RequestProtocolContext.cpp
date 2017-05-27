#include "Net/HTTP/RequestProtocolContext.h"
#include "Net/HTTP/RequestManager.h"
#include "Plugin.h"

#include <exception>

namespace SKU::Net::HTTP {
	
	void RequestProtocolContext::Initialize()
	{
		if (GetOwner() == nullptr)
		{
			Plugin::Log(LOGL_INFO, "(HTTP) RequestProtocolContext: Request context has no owner. Skipping initialization.");

			throw std::exception();
		}

		curl_last_error = CURLE_OK;
		curl_handle = curl_easy_init();

		if (curl_handle == nullptr)
		{
			throw std::bad_exception();
		}
	}

	void RequestProtocolContext::Initialize(Method method, std::string url, std::string body)
	{
		Initialize();

		this->url = url;
		this->body = body;
		this->method = method;

		Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestProtocolContext: Request (id: %d) URL=\"%s\"", GetOwner()->GetID(), url.c_str());
	}

	void RequestProtocolContext::Cleanup()
	{
		Plugin::Log(LOGL_VERBOSE, "(HTTP) RequestProtocolContext: Cleaning up request (id: %d).", GetOwner()->GetID());

		if (curl_handle != nullptr)
		{
			curl_easy_cleanup(curl_handle);
			curl_handle = nullptr;
		}
	}
}
