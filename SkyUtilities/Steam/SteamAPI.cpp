#include "SteamAPI.h"

namespace SKU::Steam {
  void SteamAPI::DeployAppIDFile()
  {
    std::ofstream appidfs("steam_appid.txt");
    appidfs << "72850";
  }

  SteamAPI::SteamAPI()
  {
    hSteamDLL = LoadLibraryA("steam_api.dll");

    if (IsValid())
    {
      Init = (SteamAPIInit_Ptr) GetProcAddress(hSteamDLL, "SteamAPI_Init");
      Shutdown = (SteamAPIShutdown_Ptr) GetProcAddress(hSteamDLL, "SteamAPI_Shutdown");

      GetUser = (SteamUserAPI_Ptr) GetProcAddress(hSteamDLL, "SteamUser");
      GetFriends = (SteamFriendsAPI_Ptr) GetProcAddress(hSteamDLL, "SteamFriends");

      if (Init == nullptr
        || Shutdown == nullptr
        || GetUser == nullptr
        || GetFriends == nullptr
        || false == Init())
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

  inline void SteamAPI::Unload()
  {
    FreeLibrary(hSteamDLL);
    hSteamDLL = nullptr;
  }
}