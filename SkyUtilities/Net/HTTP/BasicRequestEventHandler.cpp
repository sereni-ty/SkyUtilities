#include "Net/HTTP/BasicRequestEventHandler.h"

#include "Net/NetInterface.h"

#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"

#include "PapyrusEventManager.h"
#include "Plugin.h"

#include <curl/curl.h>

namespace SKU::Net::HTTP {
  void BasicRequestEventHandler::OnRequestFinished(Request::Ptr request)
  {
    Plugin::Log(LOGL_VERBOSE, "BasicRequestEventHandler: Handling request (id: %d).",
      request->GetID());

    RequestProtocolContext::Ptr ctx = request->GetProtocolContext<RequestProtocolContext>();
    PapyrusEvent::Args &args = PapyrusEvent::Args {std::make_any<int>(request->GetID()), std::make_any<bool>(request->GetState() == Request::sFailed)};

    if (request->GetState() == Request::sOK)
    {
      long response_code;
      curl_easy_getinfo(ctx->curl_handle, CURLINFO_RESPONSE_CODE, &response_code);

      args.emplace_back(std::make_any<int>(response_code));
      args.emplace_back(std::make_any<std::string>(ctx->GetResponse()));
    }
    else
    {
      args.emplace_back(std::make_any<int>(-1));
      args.emplace_back(std::make_any<std::string>());
    }

    Plugin::GetInstance()->GetPapyrusEventManager()->Send(Interface::GetEventString(Interface::evHTTPRequestFinished), std::move(args));
  }
}