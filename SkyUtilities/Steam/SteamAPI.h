#pragma once

#include <steam/steam_api.h>
#include <fstream>

namespace SKU::Steam {
  using SteamAPIInit_Ptr = bool(*)();
  using SteamAPIShutdown_Ptr = void(*)();

  using SteamUserAPI_Ptr = ISteamUser*(*)();
  using SteamFriendsAPI_Ptr = ISteamFriends*(*)();

  class SteamAPI
  {
    public:
    static void DeployAppIDFile();

    public:
    SteamAPI();
    ~SteamAPI();

    inline bool IsValid();

    private:
    inline void Unload();

    private:
    HMODULE hSteamDLL;

    SteamAPIInit_Ptr Init;
    SteamAPIShutdown_Ptr Shutdown;

    public:
    SteamUserAPI_Ptr GetUser;
    SteamFriendsAPI_Ptr GetFriends;
  };
}