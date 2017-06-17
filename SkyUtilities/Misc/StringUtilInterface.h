#pragma once

#include "Events.h"

#include <memory>

struct StaticFunctionTag;

namespace SKU::Misc::StringUtil {
  class Interface : public IEventHandler
  {
    friend class Plugin;

    public:
    using Ptr = std::unique_ptr<Interface>;

    public:
    Interface();
    ~Interface();

    public:
    static VMResultArray<BSFixedString> RegexSearch(StaticFunctionTag*, BSFixedString pattern, BSFixedString str);
    static VMResultArray<BSFixedString> RegexMatch(StaticFunctionTag*, BSFixedString pattern, BSFixedString str);
    static BSFixedString RegexReplace(StaticFunctionTag*, BSFixedString pattern, BSFixedString replacement, BSFixedString str);

    // IEventHandler
    //
    public:
    virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept final;
  };
}
