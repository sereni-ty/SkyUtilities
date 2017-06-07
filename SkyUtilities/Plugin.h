#pragma once

#include "Singleton.h"
#include "Events.h"

#include "Net/NetInterface.h"
#include "PapyrusEventManager.h"

#include <set>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct SKSEInterface;
struct PluginInfo;

enum
{
  LOGL_CRITICAL = -2,
  LOGL_WARNING,
  LOGL_INFO = 0,
  LOGL_DETAILED,
  LOGL_VERBOSE
};

#define LOGLEVEL LOGL_VERBOSE

#define PLUGIN_RELEASE_VERSION_MAJOR	0
#define PLUGIN_RELEASE_VERSION_MINOR	1

#define PLUGIN_RELEASE_VERSION_STR	"0.1"

#define PLUGIN_VERSION ((PLUGIN_RELEASE_VERSION_MAJOR & 0xFF) << 24) | (PLUGIN_RELEASE_VERSION_MINOR & 0xFFFFFF)

#define PLUGIN_NAME "SkyUtilities"

#define PLUGIN_RELATIVE_SAVES_PATH "\\My Games\\Skyrim\\Saves\\"
#define PLUGIN_RELATIVE_LOG_PATH "\\My Games\\Skyrim\\SKSE\\" PLUGIN_NAME ".log"

#define PLUGIN_SERIALIZATION_UID 'SKUS'
#define PLUGIN_SERIALIZATION_TYPE 'PLSU'
#define PLUGIN_SERIALIZATION_VERSION 1

namespace SKU {
  class Plugin : public Singleton<Plugin>
  {
    IS_SINGLETON_CLASS(Plugin)

    public:
    void Stop();

    public:
    bool OnSKSEQuery(const SKSEInterface *skse, PluginInfo *info);
    bool OnSKSELoad(const SKSEInterface *skse);

    static bool OnSKSERegisterPapyrusFunctionsProxy(VMClassRegistry*);
    static void OnSKSEMessageProxy(SKSEMessagingInterface::Message*);

    static void OnSKSESaveGameProxy(SKSESerializationInterface*);
    static void OnSKSELoadGameProxy(SKSESerializationInterface*);

    public:
    void OnNewGame();
    void OnSaveGame();
    void OnPreLoadGame();
    void OnPostLoadGame();

    public:
    bool IsActive() noexcept;

    public:
    Net::Interface::Ptr &GetNetInterface();
    PapyrusEventManager::Ptr &GetPapyrusEventManager();

    public:
    static void Log(unsigned int level, const char *fmt, ...);

    private:
    //static std::unordered_map<std::string, std::string> Configuration();

    private:
    Net::Interface::Ptr net;
    PapyrusEventManager::Ptr papyrus_event_manager;

    bool is_plugin_ready;
    bool is_game_ready;
  };
}

extern "C"
{
  bool __declspec(dllexport) SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info);
  bool __declspec(dllexport) SKSEPlugin_Load(const SKSEInterface * skse);
}
