#pragma once

#include "Singleton.h"
#include "Events.h"

#include <set>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

struct SKSEInterface;
struct PluginInfo;

enum
{
	LOGL_CRITICAL = -2,
	LOGL_WARNING,
	LOGL_INFO,
	LOGL_DETAILED,
	LOGL_VERBOSE
};

#define DEBUG LOGL_VERBOSE

#define PLUGIN_RELEASE_VERSION_MAJOR	0
#define PLUGIN_RELEASE_VERSION_MINOR	1

#define PLUGIN_RELEASE_VERSION_STR	"0.1"

#define PLUGIN_VERSION ((PLUGIN_RELEASE_VERSION_MAJOR & 0xFF) << 24) | ((PLUGIN_RELEASE_VERSION_MINOR & 0xFFFFF) << 4) | (DEBUG & 0x0F)

#define PLUGIN_NAME "SkyUtilities"

#define PLUGIN_RELATIVE_LOG_PATH "\\My Games\\Skyrim\\SKSE\\" PLUGIN_NAME ".log"
#define PLUGIN_LOG_FILENAME PLUGIN_NAME ".log"

namespace SKU {

	class Plugin : public Singleton<Plugin>/*, public IEventDistributor*/
	{
		IS_SINGLETON_CLASS(Plugin)

		public:
			bool Initialize();
			void Stop();

		public:
			bool OnSKSEQuery(const SKSEInterface *skse, PluginInfo *info);
			bool OnSKSELoad(const SKSEInterface *skse);

			static bool OnSKSERegisterPapyrusFunctionsProxy(VMClassRegistry*);
			static void OnSKSEMessageProxy(SKSEMessagingInterface::Message*);

		public:
			bool IsGameReady();

		public:
			static void Log(unsigned int level, const char *fmt, ...);

		private:
			std::set<IEventHandler*> event_handler_set;

			bool is_plugin_active;
			bool is_game_ready;

		private:
			std::set<HINSTANCE> dll_handle_set;
	};

}

extern "C"
{
	bool __declspec(dllexport) SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info);
	bool __declspec(dllexport) SKSEPlugin_Load(const SKSEInterface * skse);
}
