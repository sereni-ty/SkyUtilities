#include "Plugin.h"
#include "PapyrusEventHandler.h"

#include "Net/NetInterface.h"
#include "Net/Request.h"
#include "Net/HTTP/RequestProtocolContext.h"
#include "Net/HTTP/RequestManager.h"

#include <skse/PapyrusNativeFunctions.h>
#include <skse/GameForms.h>

#include <exception>

namespace SKU::Net { // TODO: Consider writing class with control management (Start, Stop, ..), e.g. InterfaceBase (Start, Stop, Initialize)

	Interface::Interface() 
		: stopped(false)
	{
		Plugin::Log(LOGL_INFO, "Net: Initializing.");
		Plugin::Log(LOGL_DETAILED, "Net: Enabling events");
		PapyrusEventHandler::GetInstance()->Register(GetEventString(evHTTPRequestFinished));
	}

	Interface::~Interface() 
	{
		Stop();
	}

	void Interface::Stop()
	{
		if (stopped == true)
			return;

		// events
		PapyrusEventHandler::GetInstance()->Unregister(GetEventString(evHTTPRequestFinished));

		// request managers
		HTTP::RequestManager::GetInstance()->Stop();
		
		// internal state
		stopped = true;

		Plugin::Log(LOGL_VERBOSE, "Net: Stopped.");
	}

	long Interface::HTTPGETRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, long timeout)
	{
		if (GetInstance()->stopped == true)
			return Request::sFailed;

		return Interface::HTTPRequest(form, HTTP::RequestProtocolContext::mGET, std::string(url.data), "", timeout);
	}

	long Interface::HTTPPOSTRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, BSFixedString body, long timeout)
	{
		if (GetInstance()->stopped == true)
			return Request::sFailed;

		return Interface::HTTPRequest(form, HTTP::RequestProtocolContext::mGET, std::string(url.data), std::string(body.data), timeout);
	}

	long Interface::HTTPRequest(TESForm *form, HTTP::RequestProtocolContext::Method method, std::string url, std::string body, long timeout)
	{
		if (Plugin::GetInstance()->IsGameReady() == false)	// Note: Just a precaution. Shouldn't happen: If the script was able to call a function
			return Request::sFailed;						// (well, this one in this case) SKSE should've sent a message notification and thus 
															// the method should always return true.

		try
		{
			Request::Ptr request = Request::Create<HTTP::RequestProtocolContext>();

			request->SetTimeout(timeout);
			request->GetProtocolContext<HTTP::RequestProtocolContext>()->Initialize(method, url, body);

			if(form != nullptr)
				PapyrusEventHandler::GetInstance()->AddRecipient(GetEventString(evHTTPRequestFinished), form);

			if (true == HTTP::RequestManager::GetInstance()->AddRequest(request, true))
			{
				return request->GetID();
			}
		}
		catch (std::bad_exception)
		{
			Plugin::Log(LOGL_CRITICAL, "Stopping Plugin.");
			Plugin::GetInstance()->Stop();
		}
		catch (std::exception) {}

		return Request::sFailed;
	}

	BSFixedString Interface::URLEncode(StaticFunctionTag*, BSFixedString raw)
	{
		std::string encoded;
		size_t raw_len;
		CURL *curl;

		if (raw.data != nullptr 
		&& (raw_len = strlen(raw.data)) > 0)
		{
			if((curl = curl_easy_init()) != nullptr)
			{
				char *output = curl_easy_escape(curl, raw.data, raw_len);

				if (output != nullptr) 
				{
					encoded.assign(output);
					curl_free(output);
				}

				curl_easy_cleanup(curl);
			}
			else // OOM
			{
				Plugin::Log(LOGL_CRITICAL, "Stopping Plugin.");
				Plugin::GetInstance()->Stop();
			}
		}

		return encoded.c_str();
	}

	void Interface::OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept
	{
		registry->RegisterFunction(new NativeFunction3<StaticFunctionTag, long, TESForm*, BSFixedString, long>					("HTTPGETRequest", "SKUNet", HTTPGETRequest, registry));
		registry->RegisterFunction(new NativeFunction4<StaticFunctionTag, long, TESForm*, BSFixedString, BSFixedString, long>	("HTTPPOSTRequest", "SKUNet", HTTPPOSTRequest, registry));
		registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, BSFixedString, BSFixedString>							("URLEncode", "SKUNet", URLEncode, registry));

		Plugin::Log(LOGL_DETAILED, "Net: Registered Papyrus functions.");
	}

	std::string Interface::GetEventString(PapyrusEvent event) noexcept
	{
		switch (event)
		{
			case evHTTPRequestFinished: return "OnHTTPRequestFinished";
			default: return "";
		}
	}
}
