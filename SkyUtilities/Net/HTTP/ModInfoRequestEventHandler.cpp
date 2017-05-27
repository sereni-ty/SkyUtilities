#include "Net/HTTP/ModInfoRequestEventHandler.h"

#include "Net/NetInterface.h"

#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"

#include "PapyrusEventHandler.h"
#include "Plugin.h"

#include <curl/curl.h>

namespace SKU::Net::HTTP {

	void ModInfoRequestEventHandler::OnRequestFinished(Request::Ptr request)
	{
		Plugin::Log(LOGL_VERBOSE, "ModInfoRequestEventHandler: Handling request (id: %d).",
			request->GetID());

		/*RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();
		PapyrusEvent::Args &args = PapyrusEvent::Args{ };



		PapyrusEventHandler::GetInstance()->Send(Interface::GetEventString(Interface::evModInfoRetrieval), std::move(args));*/
	}

}
