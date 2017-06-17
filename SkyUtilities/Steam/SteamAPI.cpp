#include "SteamAPI.h"
#include "Plugin.h"

namespace SKU::Steam {
  void SteamAPI::DeployAppIDFile()
  {
    std::ofstream appidfs("steam_appid.txt");

    if (appidfs.is_open() == true)
    {
      appidfs << "72850";
    }
  }

  SteamAPI::SteamAPI()
  {
    hSteamDLL = LoadLibraryA("steam_api.dll");

    if (IsValid() == true)
    {
      Init = (SteamAPIInit_Ptr) GetProcAddress(hSteamDLL, "SteamAPI_Init");
      Shutdown = (SteamAPIShutdown_Ptr) GetProcAddress(hSteamDLL, "SteamAPI_Shutdown");

      GetUser = (SteamUserAPI_Ptr) GetProcAddress(hSteamDLL, "SteamUser");
      GetStats = (SteamStatsAPI_Ptr) GetProcAddress(hSteamDLL, "SteamUserStats");
      GetFriends = (SteamFriendsAPI_Ptr) GetProcAddress(hSteamDLL, "SteamFriends");

      if (IsLoaded() == false || false == Init())
      {
        Unload();
      }
    }
  }

  SteamAPI::~SteamAPI()
  {
    if (IsValid() == true)
    {
      Shutdown();
      Unload();
    }
  }

  inline bool SteamAPI::IsValid()
  {
    return hSteamDLL != nullptr;
  }

  inline bool SteamAPI::IsLoaded()
  {
    return GetUser != nullptr
      && GetStats != nullptr
      && GetFriends != nullptr
      && Init != nullptr
      && Shutdown != nullptr;
  }

  inline void SteamAPI::Unload()
  {
    FreeLibrary(hSteamDLL);
    hSteamDLL = nullptr;

    GetUser = nullptr;
    GetStats = nullptr;
    GetFriends = nullptr;
  }
}