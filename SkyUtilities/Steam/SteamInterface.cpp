#include "Steam/SteamInterface.h"
#include "steam/SteamAPI.h"
#include "Plugin.h"

#include "SKSE/PapyrusVM.h"
#include "SKSE/PapyrusNativeFunctions.h"

#include <string>

namespace SKU::Steam::Config {
  Configuration::Setting<bool> SteamAPIEnabled {
    std::string("SteamAPI.Enabled"),
    true,
    [] (const bool&) -> bool
    {
      return true;
    }
  };
}

namespace SKU::Steam {
  Interface::Interface()
  {
    Plugin::GetInstance()->GetConfiguration()->SetInitial(Config::SteamAPIEnabled);
  }

  Interface::~Interface()
  {}

  BSFixedString Interface::CurrentSteamUserProfileID(StaticFunctionTag*)
  {
    bool not_restricted;
    std::string profile_id;

    Plugin::GetInstance()->GetConfiguration()->Get<bool>(Config::SteamAPIEnabled, not_restricted);

    if (not_restricted == true)
    {
      SteamAPI::DeployAppIDFile();
      SteamAPI api;

      if (api.IsValid() == true && api.GetUser != nullptr /*&& Plugin::GetInstance()->IsActive() == true*/)
      {
        profile_id = std::to_string(api.GetUser()->GetSteamID().ConvertToUint64());
      }
    }

    return BSFixedString(profile_id.c_str());
  }

  void Interface::OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept
  {
    registry->RegisterFunction(new NativeFunction0<StaticFunctionTag, BSFixedString>("CurrentSteamUserProfileID", "SKUSteam", CurrentSteamUserProfileID, registry));

    Plugin::Log(LOGL_DETAILED, "Steam: Registered Papyrus functions.");
  }
}