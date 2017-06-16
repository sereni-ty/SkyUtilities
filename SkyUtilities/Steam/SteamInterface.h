#pragma once

#include "Events.h"
#include "Configuration.h"

#include <memory>

struct StaticFunctionTag;
class TESForm;

namespace SKU::Steam {
  namespace Config {
    extern Configuration::Setting<bool> SteamAPIEnabled;
  }

  class Interface : public IEventHandler
  {
    friend class Plugin;

    public:
    using Ptr = std::unique_ptr<Interface>;

    public:
    Interface();
    ~Interface();

    public:
    static BSFixedString GetUserProfileID(StaticFunctionTag*);
    static BSFixedString GetUserProfileName(StaticFunctionTag*);

    static long GetNextAchievementID(StaticFunctionTag*, long i);

    static BSFixedString GetAchievementName(StaticFunctionTag*, long i);
    static bool GetAchievementStatus(StaticFunctionTag*, BSFixedString name);
    static BSFixedString GetAchievementDescription(StaticFunctionTag*, BSFixedString name);

    // IEventHandler
    //
    public:
    virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept final;
  };
}
