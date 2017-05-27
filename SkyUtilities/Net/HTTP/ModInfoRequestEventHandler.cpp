#include "Net/HTTP/ModInfoRequestEventHandler.h"

#include "Net/NetInterface.h"

#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"

#include "PapyrusEventHandler.h"
#include "Plugin.h"

#include <curl/curl.h>

namespace SKU::Net::HTTP {

	void NexusModInfoRequestEventHandler::OnRequestFinished(Request::Ptr request)
	{
		Plugin::Log(LOGL_VERBOSE, "NexusModInfoRequestEventHandler: Handling request (id: %d).",
			request->GetID());
	}

	void LLabModInfoRequestEventHandler::OnRequestFinished(Request::Ptr request)
	{
		Plugin::Log(LOGL_VERBOSE, "LLabModInfoRequestEventHandler: Handling request (id: %d).",
			request->GetID());
	}

}
