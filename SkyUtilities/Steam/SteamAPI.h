#pragma once

#include <steam/steam_api.h>
#include <fstream>

namespace SKU::Steam {
  using SteamAPIInit_Ptr = bool(*)();
  using SteamAPIShutdown_Ptr = void(*)();

  using SteamUserAPI_Ptr = ISteamUser*(*)();
  using SteamFriendsAPI_Ptr = ISteamFriends*(*)();
  using SteamStatsAPI_Ptr = ISteamUserStats*(*)();

  class SteamAPI
  {
    public:
    static void DeployAppIDFile();

    public:
    SteamAPI();
    ~SteamAPI();

    inline bool IsValid();
    inline bool IsLoaded();

    private:
    inline void Unload();

    private:
    HMODULE hSteamDLL;

    SteamAPIInit_Ptr Init;
    SteamAPIShutdown_Ptr Shutdown;

    public:
    SteamUserAPI_Ptr GetUser;
    SteamStatsAPI_Ptr GetStats;
    SteamFriendsAPI_Ptr GetFriends;
  };
}