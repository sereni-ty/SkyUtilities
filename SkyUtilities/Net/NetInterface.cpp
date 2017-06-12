#include "Plugin.h"
#include "PapyrusEventManager.h"

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

namespace SKU::Net::Config {
  Configuration::Setting<uint32_t> PapyrusCallsPerTimeFrame(
    std::string("Net.Papyrus.CallsPerTimeFrame"),
    100,
    [] (const uint32_t &value) -> uint32_t
  {
    return value > 100 ? 100 : value;
  }
  );

  Configuration::Setting<uint32_t> PapyrusMaxTransgressions(
    std::string("Net.Papyrus.MaxTransgressions"),
    3,
    [] (const uint32_t &value) -> uint32_t
  {
    return value > 3 ? 3 : value;
  }
  );

  Configuration::Setting<uint32_t> PapyrusTimeFrame(
    std::string("Net.Papyrus.TimeFrame"),
    1000,
    [] (const uint32_t &value) -> uint32_t
  {
    return value > 1000 ? 1000 : value;
  }
  );

  Configuration::Setting<long> RequestsDefaultTimeout(
    std::string("Net.Requests.DefaultTimeout"),
    2500,
    [] (const long &value) -> long
  {
    return value > 2500 || value <= 0 ? 2500 : value;
  }
  );
}

namespace SKU::Net {
  Interface::Interface()
  {
    Plugin::GetInstance()->GetConfiguration()->SetInitial<uint32_t>(Config::PapyrusCallsPerTimeFrame);
    Plugin::GetInstance()->GetConfiguration()->SetInitial<uint32_t>(Config::PapyrusMaxTransgressions);
    Plugin::GetInstance()->GetConfiguration()->SetInitial<uint32_t>(Config::PapyrusTimeFrame);
    Plugin::GetInstance()->GetConfiguration()->SetInitial<long>(Config::RequestsDefaultTimeout);
  }

  Interface::~Interface()
  {}

  void Interface::Start()
  {
    Plugin::Log(LOGL_INFO, "Net: Initializing.");

    Plugin::GetInstance()->GetPapyrusEventManager()->Register(GetEventString(evHTTPRequestFinished));
    Plugin::GetInstance()->GetPapyrusEventManager()->Register(GetEventString(evModInfoRetrieval));

    if (http_requestmanager == nullptr)
    {
      http_requestmanager = std::make_unique<HTTP::RequestManager>();
    }

    http_requestmanager->Start(); // in case there were requests loaded.
  }

  void Interface::Stop()
  {
    Plugin::Log(LOGL_VERBOSE, "Net: Stopping.");

    // events
    Plugin::Log(LOGL_VERBOSE, "Net: Removing Events.");
    Plugin::GetInstance()->GetPapyrusEventManager()->Unregister(GetEventString(evModInfoRetrieval));
    Plugin::GetInstance()->GetPapyrusEventManager()->Unregister(GetEventString(evHTTPRequestFinished));

    // request manager
    this->http_requestmanager = std::make_unique<HTTP::RequestManager>();

    // internal state
    Plugin::Log(LOGL_VERBOSE, "Net: Stopped.");
  }

  HTTP::RequestManager::Ptr &Interface::GetHTTPRequestManager()
  {
    return http_requestmanager;
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
    return Interface::HTTPRequest(
      HTTP::NexusModInfoRequestEventHandler::TypeID,
      form,
      HTTP::RequestProtocolContext::mGET,
      std::string("www.nexusmods.com/skyrim/mods/" + std::string(mod_id.data)) + "/?",
      "",
      -1
    );
  }

  long Interface::GetLLabModInfo(StaticFunctionTag*, TESForm *form, BSFixedString mod_id)
  {
    return Interface::HTTPRequest(
      HTTP::LLabModInfoRequestEventHandler::TypeID,
      form,
      HTTP::RequestProtocolContext::mGET,
      std::string("www.loverslab.com/files/file/" + std::string(mod_id.data)) + "-",
      "",
      -1
    );
  }

  long Interface::HTTPRequest(uint32_t request_handler_type_id, TESForm *form, HTTP::RequestProtocolContext::Method method, std::string url, std::string body, long timeout)
  {
    Configuration::Ptr &conf = Plugin::GetInstance()->GetConfiguration();

    if (timeout != Config::RequestsDefaultTimeout(timeout))
    {
      conf->Get<long>(Config::RequestsDefaultTimeout, timeout);
    }

    // TODO: check if scripts are paused, too..
    // TODO: own class for script call frequency check and blacklist..
    using namespace std::chrono;
    struct ScriptCallsTimeInformation
    {
      std::vector<steady_clock::time_point> last_known_calls;

      uint8_t limit_exceeding_count;
    };

    static std::unordered_set<TESForm *> script_blacklist;
    static std::unordered_map<TESForm *, ScriptCallsTimeInformation> script_calls;

    steady_clock::time_point current_time = steady_clock::now();

    if (Plugin::GetInstance()->IsActive() == false)
    {
      return Request::sFailed;
    }

    if (script_blacklist.find(form) != script_blacklist.end())
    {
      return Request::sBlacklisted;
    }

    if (script_calls.find(form) != script_calls.end())
    {
      ScriptCallsTimeInformation &info = script_calls.at(form);
      info.last_known_calls.push_back(current_time);

      uint32_t timeframe, calls_per_timeframe, max_transgressions;

      conf->Get<uint32_t>(Config::PapyrusTimeFrame, timeframe);
      conf->Get<uint32_t>(Config::PapyrusCallsPerTimeFrame, calls_per_timeframe);
      conf->Get<uint32_t>(Config::PapyrusMaxTransgressions, max_transgressions);

      Plugin::Log(LOGL_VERBOSE, "Net: Loaded Configuration Values (%d %d %d)", timeframe, calls_per_timeframe, max_transgressions); // TODO: remove this one

      if (info.last_known_calls.size() == calls_per_timeframe)
      {
        while (duration_cast<milliseconds>(current_time - info.last_known_calls.back()).count() < timeframe)
        {
          info.last_known_calls.pop_back();
        }

        if (info.last_known_calls.size() == 0)
        {
          if (++info.limit_exceeding_count <= max_transgressions)
          {
            Plugin::Log(LOGL_INFO, "Net: A specific script exceeded the request limit of %d in a time frame of %dms. Limit exceeded %d times.",
              calls_per_timeframe, timeframe, info.limit_exceeding_count);
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

    if (Plugin::GetInstance()->IsActive() == false)	// Note: Just a precaution. Shouldn't happen: If the script was able to call a function
    {													// (well, this one in this case) SKSE should've sent a message notification and thus
      return Request::sFailed;						// the method should always return true.
    }

    try
    {
      Request::Ptr request = Request::Create<HTTP::RequestProtocolContext>();

      request->SetTimeout(timeout);
      request->GetProtocolContext<HTTP::RequestProtocolContext>()->Initialize(method, url, body);

      if (HTTP::LLabModInfoRequestEventHandler::TypeID < request_handler_type_id
        || true == Plugin::GetInstance()->GetNetInterface()->http_requestmanager->AddRequest(request))
      {
        if (form != nullptr)
        {
          switch (request_handler_type_id)
          {
            case HTTP::BasicRequestEventHandler::TypeID:
            {
              request->SetHandler(std::make_shared<HTTP::BasicRequestEventHandler>());
              Plugin::GetInstance()->GetPapyrusEventManager()->AddRecipient(GetEventString(evHTTPRequestFinished), form);
            } break;

            case HTTP::NexusModInfoRequestEventHandler::TypeID:
            {
              request->SetHandler(std::make_shared<HTTP::NexusModInfoRequestEventHandler>());
              Plugin::GetInstance()->GetPapyrusEventManager()->AddRecipient(GetEventString(evModInfoRetrieval), form);
            } break;

            case HTTP::LLabModInfoRequestEventHandler::TypeID:
            {
              request->SetHandler(std::make_shared<HTTP::LLabModInfoRequestEventHandler>());
              Plugin::GetInstance()->GetPapyrusEventManager()->AddRecipient(GetEventString(evModInfoRetrieval), form);
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

  void Interface::Serialize(std::stack<ISerializeable::SerializationEntity> &serialized_entities)
  {
    http_requestmanager->Serialize(serialized_entities);
  }

  void Interface::Deserialize(ISerializeable::SerializationEntity &serialized)
  {
    http_requestmanager->Deserialize(serialized);
  }

  bool Interface::IsRequestedSerialization(const ISerializeable::SerializationEntity &serialized)
  {
    http_requestmanager = std::make_unique<HTTP::RequestManager>();

    return http_requestmanager->IsRequestedSerialization(serialized);
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