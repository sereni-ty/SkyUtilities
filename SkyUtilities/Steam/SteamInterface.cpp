#include "Steam/SteamInterface.h"
#include "steam/SteamAPI.h"
#include "Plugin.h"

#include "SKSE/PapyrusVM.h"
#include "SKSE/PapyrusNativeFunctions.h"

#include <string>
#include <mutex>
#include <thread>
#include <future>

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

namespace SKU::Steam { // TODO: achievements buffern..
  const long InvalidAchievementID = -1;

  struct Achievement
  {
    std::string name, description;
    bool status;

    Achievement()
      : name(""), description(""), status(false)
    {}

    Achievement(std::string n, std::string d, bool s)
      : name(n), description(d), status(s)
    {
      Plugin::Log(LOGL_VERBOSE, "Achievement: %s ('%s'): %s", n.c_str(), d.c_str(), s ? "done" : "undone");
    }
  };

  std::vector<Achievement> achievements;
  std::map<std::string, long> achievement_indices;

  std::mutex achievement_retrieval_mtx;
  std::future<long> achievement_retrieval_startup_future;

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

  inline long RetrieveAchievements()
  {
    std::unique_lock<std::mutex> scope_lock(achievement_retrieval_mtx);

    if (achievements.empty() == false)
    {
      return achievements.size();
    }

    SteamAPI::DeployAppIDFile();
    SteamAPI api;

    if (IsSteamAPIAccessible(api) == false)
    {
      return -1;
    }

    size_t num_achievements = api.GetStats()->GetNumAchievements();
    achievements.resize(num_achievements);

    Plugin::Log(LOGL_INFO, "Steam: %d achievements..", achievements.capacity());

    for (size_t i = 0; i < num_achievements; i++)
    {
      std::string name = api.GetStats()->GetAchievementName(i);
      std::string description = api.GetStats()->GetAchievementDisplayAttribute(name.c_str(), "desc");

      bool status = false;
      api.GetStats()->GetAchievement(name.c_str(), &status);

      achievement_indices[name] = i;
      achievements[i] = Achievement(name, description, status);
    }

    return achievements.capacity();
  }

  Interface::Interface()
  {
    Plugin::GetInstance()->GetConfiguration()->SetInitial(Config::SteamAPIEnabled);

    achievement_retrieval_startup_future = std::move(std::async(RetrieveAchievements));
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
    if (RetrieveAchievements() <= 0)
    {
      return InvalidAchievementID;
    }

    i = (i < 0) ? 0 : 1 + i;

    if (achievements.size() <= i)
    {
      return InvalidAchievementID;
    }

    return i;
  }

  BSFixedString Interface::GetAchievementName(StaticFunctionTag*, long i)
  {
    if (RetrieveAchievements() <= 0 || achievements.size() <= i || i < 0)
    {
      return "";
    }

    return achievements[i].name.c_str();
  }

  bool Interface::GetAchievementStatus(StaticFunctionTag*, BSFixedString name)
  {
    if (RetrieveAchievements() <= 0)
    {
      return InvalidAchievementID;
    }

    auto achievement_index = achievement_indices.find(name.data);

    if (achievement_index != achievement_indices.end())
    {
      return achievements[(*achievement_index).second].status;
    }

    return InvalidAchievementID;
  }

  BSFixedString Interface::GetAchievementDescription(StaticFunctionTag*, BSFixedString name)
  {
    if (RetrieveAchievements() <= 0)
    {
      return "";
    }

    auto achievement_index = achievement_indices.find(name.data);

    if (achievement_index != achievement_indices.end())
    {
      return achievements[(*achievement_index).second].description.c_str();
    }

    return "";
  }

  void Interface::OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept
  {
    registry->RegisterFunction(new NativeFunction0<StaticFunctionTag, BSFixedString>("GetSteamUserProfileID", "SKUSteam", GetUserProfileID, registry));
    registry->RegisterFunction(new NativeFunction0<StaticFunctionTag, BSFixedString>("GetSteamUserProfileName", "SKUSteam", GetUserProfileName, registry));

    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, long, long>("GetNextAchievementID", "SKUSteam", GetNextAchievementID, registry));
    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, BSFixedString, long>("GetAchievementName", "SKUSteam", GetAchievementName, registry));
    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, bool, BSFixedString>("GetAchievementStatus", "SKUSteam", GetAchievementStatus, registry));
    registry->RegisterFunction(new NativeFunction1<StaticFunctionTag, BSFixedString, BSFixedString>("GetAchievementDescription", "SKUSteam", GetAchievementDescription, registry));

    registry->SetFunctionFlags("SKUSteam", "GetNextAchievementID", VMClassRegistry::kFunctionFlag_NoWait);
    registry->SetFunctionFlags("SKUSteam", "GetAchievementName", VMClassRegistry::kFunctionFlag_NoWait);
    registry->SetFunctionFlags("SKUSteam", "GetAchievementStatus", VMClassRegistry::kFunctionFlag_NoWait);
    registry->SetFunctionFlags("SKUSteam", "GetAchievementDescription", VMClassRegistry::kFunctionFlag_NoWait);

    Plugin::Log(LOGL_DETAILED, "Steam: Registered Papyrus functions.");
  }
}