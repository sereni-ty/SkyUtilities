#include "Plugin.h"
#include "PapyrusEvents.h"

#include "Net/Interface.h"
#include "Net/Request.h"
#include "Net/HTTP/RequestProtocolContext.h"
#include "Net/HTTP/RequestManager.h"

#include <skse/PapyrusNativeFunctions.h>
#include <skse/GameForms.h>

#include <exception>

namespace SKU { namespace Net { // TODO: Consider writing class with control management (Start, Stop, ..), e.g. InterfaceBase (Start, Stop, Initialize)

	Interface::Interface() 
		: stopped(false)
	{
		Plugin::Log(LOGL_INFO, "Net: Setting up SKSE mod events.");
	}

	Interface::~Interface() 
	{
		Stop();
	}

	void Interface::Stop()
	{
		if (stopped == true)
			return;

		HTTP::RequestManager::GetInstance()->Stop();
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

			SKU::PapyrusEvent::RegisterListener(GetEventString(evRequestFinished), form);

			if (true == HTTP::RequestManager::GetInstance()->AddRequest(request, true))
			{
				return request->GetID();
			}
		}
		catch (std::bad_exception)
		{
			Plugin::Log(LOGL_CRITICAL, "Stopping Plugin");
			Plugin::GetInstance()->Stop();
		}
		catch (std::exception) {}

		return Request::sFailed;
	}

	void Interface::OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry)
	{
		registry->RegisterFunction(new NativeFunction3<StaticFunctionTag, long, TESForm*, BSFixedString,				long>("HTTPGETRequest", "SKUNet", HTTPGETRequest, registry));
		registry->RegisterFunction(new NativeFunction4<StaticFunctionTag, long, TESForm*, BSFixedString, BSFixedString,	long>("HTTPPOSTRequest", "SKUNet", HTTPPOSTRequest, registry));

		Plugin::Log(LOGL_VERBOSE, "Net: Registered Papyrus functions.");
	}

	std::string Interface::GetEventString(PapyrusEvent event)
	{
		switch (event)
		{
			case evRequestFinished: return "OnRequestFinished"; // return "SKUNet.RequestFinished";
			default: return "";
		}
	}
}}
