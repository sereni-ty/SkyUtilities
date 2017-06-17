#include "StringUtilInterface.h"
#include "Plugin.h"

#include "SKSE/PapyrusVM.h"
#include "SKSE/PapyrusNativeFunctions.h"

#include <string>
#include <regex>
#include <future>
#include <thread>

namespace SKU::Misc::StringUtil {
  Interface::Interface()
  {}

  Interface::~Interface()
  {}

  VMResultArray<BSFixedString> Interface::RegexSearch(StaticFunctionTag*, BSFixedString pattern, BSFixedString str)
  {
    VMResultArray<BSFixedString> pres;
    std::smatch m;
    std::string s = str.data;

    bool stop = false;

    std::future<void> task = std::async([&] ()
    {
      try
      {
        while (stop == false && std::regex_search(s, m, std::regex(pattern.data)) == true)
        {
          for (auto match : m)
          {
            pres.push_back(match.str().c_str());
          }

          s = m.suffix().str();
        }
      }
      catch (std::exception &e)
      {
        Plugin::Log(LOGL_VERBOSE, "StringUtil: Exception on RegexSearch (%s).",
          e.what());
      }
    });

    uint32_t blocking_limit;
    Plugin::GetInstance()->GetConfiguration()->Get<uint32_t>(Config::InterfaceProcessingTimeLimit, blocking_limit);

    task.wait_for(std::chrono::milliseconds(blocking_limit));
    stop = true;

    return pres;
  }

  VMResultArray<BSFixedString> Interface::RegexMatch(StaticFunctionTag*, BSFixedString pattern, BSFixedString str)
  {
    VMResultArray<BSFixedString> pres;
    std::smatch m;
    std::string s = str.data;

    std::future<void> task = std::async([&] ()
    {
      try
      {
        if (std::regex_match(s, m, std::regex(pattern.data)) == true)
        {
          for (auto match : m)
          {
            pres.push_back(match.str().c_str());
          }
        }
      }
      catch (std::exception &e)
      {
        Plugin::Log(LOGL_VERBOSE, "StringUtil: Exception on RegexMatch (%s).",
          e.what());
      }
    });

    uint32_t blocking_limit;
    Plugin::GetInstance()->GetConfiguration()->Get<uint32_t>(Config::InterfaceProcessingTimeLimit, blocking_limit);

    Plugin::Log(LOGL_VERBOSE, "StringUtil: %dms blocking limit", blocking_limit);

    task.wait_for(std::chrono::milliseconds(blocking_limit));

    return pres;
  }

  BSFixedString Interface::RegexReplace(StaticFunctionTag*, BSFixedString pattern, BSFixedString replacement, BSFixedString str)
  {
    BSFixedString pres;

    std::future<void> task = std::async([&] ()
    {
      try
      {
        std::string result = std::regex_replace(str.data, std::regex(pattern.data), replacement.data);

        if (result.empty() == false)
        {
          pres = BSFixedString(result.c_str());
        }
      }
      catch (std::exception &e)
      {
        Plugin::Log(LOGL_VERBOSE, "StringUtil: Exception on RegexReplace (%s).",
          e.what());
      }
    });

    uint32_t blocking_limit;
    Plugin::GetInstance()->GetConfiguration()->Get<uint32_t>(Config::InterfaceProcessingTimeLimit, blocking_limit);

    task.wait_for(std::chrono::milliseconds(blocking_limit));

    return pres;
  }

  void Interface::OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept
  {
    registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, VMResultArray<BSFixedString>, BSFixedString, BSFixedString>("RegexSearch", "SKUStringUtil", RegexSearch, registry));
    registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, VMResultArray<BSFixedString>, BSFixedString, BSFixedString>("RegexMatch", "SKUStringUtil", RegexMatch, registry));
    registry->RegisterFunction(new NativeFunction3<StaticFunctionTag, BSFixedString, BSFixedString, BSFixedString, BSFixedString>("RegexReplace", "SKUStringUtil", RegexReplace, registry));

    registry->SetFunctionFlags("SKUStringUtil", "RegexSearch", VMClassRegistry::kFunctionFlag_NoWait);
    registry->SetFunctionFlags("SKUStringUtil", "RegexMatch", VMClassRegistry::kFunctionFlag_NoWait);
    registry->SetFunctionFlags("SKUStringUtil", "RegexReplace", VMClassRegistry::kFunctionFlag_NoWait);

    Plugin::Log(LOGL_DETAILED, "StringUtil: Registered Papyrus functions.");
  }
}