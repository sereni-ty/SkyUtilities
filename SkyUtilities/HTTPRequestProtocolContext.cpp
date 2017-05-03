#include "HTTPRequestProtocolContext.h"
#include "HTTPRequestManager.h"
#include "Plugin.h"

#include <exception>

namespace SKU { namespace Net { namespace HTTP {
	
	void RequestProtocolContext::Initialize(Method method, std::string url, std::string body)
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

		this->url = url;
		this->body = body;
		this->method = method;

		Plugin::Log(LOGL_VERBOSE, "(RequestProtocolContext::Initialize): request (id: %d) URL=\"%s\"", GetOwner()->GetID(), url.c_str());
	}

	void RequestProtocolContext::Cleanup()
	{
		if (curl_handle != nullptr)
		{
			curl_easy_cleanup(curl_handle);
			curl_handle = nullptr;
		}
	}

}}}
