#include "Plugin.h"
#include "PapyrusEventHandler.h"

#include "Net/NetInterface.h"
#include "Net/RequestEventHandler.h"
#include "Net/Request.h"

#include "Net/HTTP/BasicRequestEventHandler.h"
#include "Net/HTTP/ModInfoRequestEventHandler.h"
#include "Net/HTTP/RequestProtocolContext.h"
#include "Net/HTTP/RequestManager.h"

#include <skse/PapyrusNativeFunctions.h>
#include <skse/GameForms.h>

#include <exception>
#include <chrono>
#include <vector>

namespace SKU::Net { // TODO: Consider writing class with control management (Start, Stop, ..), e.g. InterfaceBase (Start, Stop, Initialize)
  Interface::Interface()
    : stopped(false)
  {
    Plugin::Log(LOGL_INFO, "Net: Initializing.");
    Plugin::Log(LOGL_DETAILED, "Net: Enabling events");

    PapyrusEventHandler::GetInstance()->Register(GetEventString(evHTTPRequestFinished));
    PapyrusEventHandler::GetInstance()->Register(GetEventString(evModInfoRetrieval));
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
    PapyrusEventHandler::GetInstance()->Unregister(GetEventString(evModInfoRetrieval));
    PapyrusEventHandler::GetInstance()->Unregister(GetEventString(evHTTPRequestFinished));

    // request managers
    HTTP::RequestManager::GetInstance()->Stop();

    // internal state
    stopped = true;

    Plugin::Log(LOGL_VERBOSE, "Net: Stopped.");
  }

  long Interface::HTTPGETRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, long timeout)
  {
    return Interface::HTTPRequest(HTTP::BasicRequestEventHandler::TypeID, form, HTTP::RequestProtocolContext::mGET, std::string(url.data), "", timeout);
  }

  long Interface::HTTPPOSTRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, BSFixedString body, long timeout)
  {
    return Interface::HTTPRequest(HTTP::BasicRequestEventHandler::TypeID, form, HTTP::RequestProtocolContext::mGET, std::string(url.data), std::string(body.data), timeout);
  }

  long Interface::GetNexusModInfo(StaticFunctionTag*, TESForm *form, BSFixedString mod_id)
  {
    return Interface::HTTPRequest(HTTP::NexusModInfoRequestEventHandler::TypeID, form, HTTP::RequestProtocolContext::mGET, std::string("www.nexusmods.com/skyrim/mods/" + std::string(mod_id.data)) + "/?", "", 25000); // TODO: add default timeout
  }

  long Interface::GetLLabModInfo(StaticFunctionTag*, TESForm *form, BSFixedString mod_id)
  {
    return Interface::HTTPRequest(HTTP::LLabModInfoRequestEventHandler::TypeID, form, HTTP::RequestProtocolContext::mGET, std::string("www.loverslab.com/files/file/" + std::string(mod_id.data)) + "-", "", 25000); // TODO: add default timeout
  }

  long Interface::HTTPRequest(uint32_t request_handler_type_id, TESForm *form, HTTP::RequestProtocolContext::Method method, std::string url, std::string body, long timeout)
  {
    using namespace std::chrono;

    struct ScriptCallsTimeInformation
    {
      std::vector<steady_clock::time_point> last_known_calls;

      uint8_t limit_exceeding_count;
    };

    static std::unordered_set<TESForm *> script_blacklist;
    static std::unordered_map<TESForm *, ScriptCallsTimeInformation> script_calls;

    steady_clock::time_point current_time = steady_clock::now();

    if (GetInstance()->stopped == true)
      return Request::sFailed;

    if (script_blacklist.find(form) != script_blacklist.end())
    {
      return Request::sBlacklisted;
    }

    if (script_calls.find(form) != script_calls.end())
    {
      ScriptCallsTimeInformation &info = script_calls.at(form);
      info.last_known_calls.push_back(current_time);

      if (info.last_known_calls.size() == REQUESTS_LIMIT_PER_TIMEFRAME)
      {
        while (duration_cast<milliseconds>(current_time - info.last_known_calls.back()).count() < REQUESTS_LIMIT_TIMEFRAME)
        {
          info.last_known_calls.pop_back();
        }

        if (info.last_known_calls.size() == 0)
        {
          if (++info.limit_exceeding_count <= REQUESTS_LIMIT_EXCEEDINGS_PERMITTED)
          {
            Plugin::Log(LOGL_INFO, "Net: A specific script exceeded the request limit of %d in a time frame of %dms. Limit exceeded %d times.",
              REQUESTS_LIMIT_PER_TIMEFRAME, REQUESTS_LIMIT_TIMEFRAME, info.limit_exceeding_count);
          }
          else
          {
            Plugin::Log(LOGL_INFO, "Net: A specific script exceeded the request limit too often and was blacklisted.");
            script_blacklist.emplace(form);
            return Request::sBlacklisted;
          }
        }
        else
        {
          info.last_known_calls.clear();
        }
      }
    }
    else
    {
      ScriptCallsTimeInformation info;
      info.limit_exceeding_count = 0;

      script_calls.emplace(form, std::move(info));
    }

    if (Plugin::GetInstance()->IsGameReady() == false)	// Note: Just a precaution. Shouldn't happen: If the script was able to call a function
    {													// (well, this one in this case) SKSE should've sent a message notification and thus
      return Request::sFailed;						// the method should always return true.
    }

    try
    {
      Request::Ptr request = Request::Create<HTTP::RequestProtocolContext>();

      request->SetTimeout(timeout);
      request->GetProtocolContext<HTTP::RequestProtocolContext>()->Initialize(method, url, body);

      if (HTTP::LLabModInfoRequestEventHandler::TypeID < request_handler_type_id // TODO: Fix this whole TypeID mess.
        || true == HTTP::RequestManager::GetInstance()->AddRequest(request))
      {
        if (form != nullptr)
        {
          switch (request_handler_type_id)
          {
            case HTTP::BasicRequestEventHandler::TypeID:
            {
              request->SetHandler(std::make_shared<HTTP::BasicRequestEventHandler>());
              PapyrusEventHandler::GetInstance()->AddRecipient(GetEventString(evHTTPRequestFinished), form);
            } break;

            case HTTP::NexusModInfoRequestEventHandler::TypeID:
            {
              request->SetHandler(std::make_shared<HTTP::NexusModInfoRequestEventHandler>());
              PapyrusEventHandler::GetInstance()->AddRecipient(GetEventString(evModInfoRetrieval), form);
            } break;

            case HTTP::LLabModInfoRequestEventHandler::TypeID:
            {
              request->SetHandler(std::make_shared<HTTP::LLabModInfoRequestEventHandler>());
              PapyrusEventHandler::GetInstance()->AddRecipient(GetEventString(evModInfoRetrieval), form);
            } break;

            default: throw std::exception();
          }
        }

        return request->GetID();
      }
    }
    catch (std::bad_exception)
    {
      Plugin::Log(LOGL_CRITICAL, "Stopping Plugin.");
      Plugin::GetInstance()->Stop();
    }
    catch (std::exception)
    {
    }

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
      if ((curl = curl_easy_init()) != nullptr)
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
        Plugin::Log(LOGL_CRITICAL, "OOM. Stopping Plugin.");
        Plugin::GetInstance()->Stop();
      }
    }

    return encoded.c_str();
  }

  BSFixedString Interface::URLDecode(StaticFunctionTag*, BSFixedString encoded)
  {
    std::string decoded;
    size_t encoded_len;
    CURL *curl;

    if (encoded.data != nullptr
      && (encoded_len = strlen(encoded.data)) > 0)
    {
      if ((curl = curl_easy_init()) != nullptr)
      {
        char *output = curl_easy_escape(curl, encoded.data, encoded_len);

        if (output != nullptr)
        {
          decoded.assign(output);
          curl_free(output);
        }

        curl_easy_cleanup(curl);
      }
      else // OOM
      {
        Plugin::Log(LOGL_CRITICAL, "OOM. Stopping Plugin.");
        Plugin::GetInstance()->Stop();
      }
    }

    return decoded.c_str();
  }

  void Interface::OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept
  {
    registry->RegisterFunction(new NativeFunction3<StaticFunctionTag, long, TESForm*, BSFixedString, long>("HTTPGETRequest", "SKUNet", HTTPGETRequest, registry));
    registry->RegisterFunction(new NativeFunction4<StaticFunctionTag, long, TESForm*, BSFixedString, BSFixedString, long>("HTTPPOSTRequest", "SKUNet", HTTPPOSTRequest, registry));

    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, BSFixedString, BSFixedString>("URLEncode", "SKUNet", URLEncode, registry));
    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, BSFixedString, BSFixedString>("URLDecode", "SKUNet", URLDecode, registry));

    registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, long, TESForm*, BSFixedString>("GetNexusModInfo", "SKUNet", GetNexusModInfo, registry));
    registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, long, TESForm*, BSFixedString>("GetLLabModInfo", "SKUNet", GetLLabModInfo, registry));

    Plugin::Log(LOGL_DETAILED, "Net: Registered Papyrus functions.");
  }

  inline std::string Interface::GetEventString(PapyrusEvent event) noexcept
  {
    switch (event)
    {
      case evHTTPRequestFinished: return "OnHTTPRequestFinished";
      case evModInfoRetrieval:	return "OnModInfoRetrieval";
      default: return "";
    }
  }
}