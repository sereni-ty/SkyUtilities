#include "StringUtilInterface.h"
#include "Plugin.h"

#include "SKSE/PapyrusVM.h"
#include "SKSE/PapyrusNativeFunctions.h"

#include <string>
#include <regex>

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

    while (std::regex_search(s, m, std::regex(pattern.data)) == true)
    {
      for (auto match : m)
      {
        pres.push_back(match.str().c_str());
      }

      s = m.suffix().str();
    }

    return pres;
  }

  VMResultArray<BSFixedString> Interface::RegexMatch(StaticFunctionTag*, BSFixedString pattern, BSFixedString str)
  {
    VMResultArray<BSFixedString> pres;
    std::smatch m;
    std::string s = str.data;

    if (std::regex_match(s, m, std::regex(pattern.data)) == true)
    {
      for (auto match : m)
      {
        pres.push_back(match.str().c_str());
      }
    }

    return pres;
  }
  BSFixedString Interface::RegexReplace(StaticFunctionTag*, BSFixedString pattern, BSFixedString replacement, BSFixedString str)
  {
    BSFixedString pres;

    std::string result = std::regex_replace(str.data, std::regex(pattern.data), replacement.data);

    if (result.empty() == false)
    {
      pres = BSFixedString(result.c_str());
    }

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