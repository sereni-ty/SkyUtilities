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
  const long InvalidAchievementID = -1;

  inline bool IsSteamAPIAccessible(SteamAPI &api)
  {
    bool not_restricted;

    Plugin::GetInstance()->GetConfiguration()->Get<bool>(Config::SteamAPIEnabled, not_restricted);

    if (api.IsLoaded() == true && not_restricted == true)
    {
      return true;
    }

    return false;
  }

  Interface::Interface()
  {
    Plugin::GetInstance()->GetConfiguration()->SetInitial(Config::SteamAPIEnabled);

    SteamAPI::DeployAppIDFile();
  }

  Interface::~Interface()
  {}

  BSFixedString Interface::GetUserProfileID(StaticFunctionTag*)
  {
    SteamAPI api;

    if (IsSteamAPIAccessible(api) == false)
    {
      return "";
    }

    return std::to_string(api.GetUser()->GetSteamID().ConvertToUint64()).c_str();
  }

  BSFixedString Interface::GetUserProfileName(StaticFunctionTag*)
  {
    SteamAPI api;

    if (IsSteamAPIAccessible(api) == false)
    {
      return "";
    }

    return api.GetFriends()->GetPersonaName();
  }

  long Interface::GetNextAchievementID(StaticFunctionTag*, long i)
  {
    SteamAPI api;

    if (IsSteamAPIAccessible(api) == false)
    {
      return InvalidAchievementID;
    }

    i = (i < 0) ? 0 : 1 + i;

    if (api.GetStats()->GetNumAchievements() <= i)
    {
      return InvalidAchievementID;
    }

    return i;
  }

  BSFixedString Interface::GetAchievementName(StaticFunctionTag*, long i)
  {
    SteamAPI api;

    if (IsSteamAPIAccessible(api) == false)
    {
      return "";
    }

    if (api.GetStats()->GetNumAchievements() <= i)
    {
      return "";
    }

    return api.GetStats()->GetAchievementName(i); // TODO: Check if that's okay if a nullptr is returned
  }

  bool Interface::GetAchievementStatus(StaticFunctionTag*, BSFixedString name)
  {
    SteamAPI api;

    if (IsSteamAPIAccessible(api) == false)
    {
      return InvalidAchievementID;
    }

    bool status = false;
    api.GetStats()->GetAchievement(name.data, &status);

    return status;
  }

  BSFixedString Interface::GetAchievementDescription(StaticFunctionTag*, BSFixedString name)
  {
    SteamAPI api;

    if (IsSteamAPIAccessible(api) == false)
    {
      return "";
    }

    return api.GetStats()->GetAchievementDisplayAttribute(name.data, "desc");
  }

  void Interface::OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept
  {
    registry->RegisterFunction(new NativeFunction0<StaticFunctionTag, BSFixedString>("GetSteamUserProfileID", "SKUSteam", GetUserProfileID, registry));
    registry->RegisterFunction(new NativeFunction0<StaticFunctionTag, BSFixedString>("GetSteamUserProfileName", "SKUSteam", GetUserProfileName, registry));

    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, long, long>("GetNextAchievementID", "SKUSteam", GetNextAchievementID, registry));
    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, BSFixedString, long>("GetAchievementName", "SKUSteam", GetAchievementName, registry));
    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, bool, BSFixedString>("GetAchievementStatus", "SKUSteam", GetAchievementStatus, registry));
    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, BSFixedString, BSFixedString>("GetAchievementDescription", "SKUSteam", GetAchievementDescription, registry));

    Plugin::Log(LOGL_DETAILED, "Steam: Registered Papyrus functions.");
  }
}