#include "Plugin.h"

#include "PapyrusEventHandler.h"

#include "Net/NetInterface.h"
#include "Storage/StorageInterface.h"

#include <common/IDebugLog.h>
#include <skse/PluginAPI.h>
#include <skse/skse_version.h>
#include <skse/PapyrusArgs.h>

#include <shlobj.h>

#include <mutex>
#include <list>

// TODO: create test classes to test if TEST macro is set just after "game ready" has been set

namespace SKU {

	Plugin::Plugin()
		: is_game_ready(false), is_plugin_active(true)
	{
		Log(LOGL_INFO, "%s (%s)\n", PLUGIN_NAME, PLUGIN_RELEASE_VERSION_STR);
	}

	Plugin::~Plugin()
	{
		Stop();

		Log(LOGL_INFO, "Plugin stopped.");
	}

	bool Plugin::Initialize()
	{
		Log(LOGL_INFO, "Initializing.");

		//
		// Event handler
		event_handler_set.emplace(Net::Interface::GetInstance());
		event_handler_set.emplace(Storage::Interface::GetInstance());

		return true;
	}

	void Plugin::Stop()
	{
		if (is_plugin_active == false)
			return;

		is_plugin_active = false;

		event_handler_set.clear();

		Log(LOGL_VERBOSE, "Stopping Interfaces: ");
		Log(LOGL_VERBOSE, " - Net");
		Net::Interface::GetInstance()->Stop();		
	}

	bool Plugin::OnSKSEQuery(const SKSEInterface *skse, PluginInfo *info)
	{
		if (skse == nullptr || info == nullptr)
			return false;
		
		if (skse->runtimeVersion < RUNTIME_VERSION_1_9_32_0)
		{
			Log(LOGL_INFO, "SKSE version is below 1.9.32.0. Get at least that version or a newer one. Stopping plugin.");
			return false;
		}

		info->name = PLUGIN_NAME;
		info->version = PLUGIN_VERSION;
		info->infoVersion = PluginInfo::kInfoVersion;

		return true;
	}

	bool Plugin::OnSKSELoad(const SKSEInterface *skse)
	{
		if (skse == nullptr)
			return false;

		SKSEPapyrusInterface *skse_papyrus = reinterpret_cast<SKSEPapyrusInterface*>(skse->QueryInterface(kInterface_Papyrus));
		SKSEMessagingInterface *skse_messaging = reinterpret_cast<SKSEMessagingInterface*>(skse->QueryInterface(kInterface_Messaging));

		if (skse_papyrus == nullptr || skse_messaging == nullptr)
		{
			Log(LOGL_INFO, "SKSE failed to return valid interfaces. Stopping plugin.");
			Log(LOGL_DETAILED, "Interfaces:\n-\tPapyrus Interface: %s\n-\tMessaging: %s", 
				(skse_papyrus ? "valid" : "invalid"),
				(skse_messaging ? "valid" : "invalid"));

			return false;
		}

		return 
				skse_papyrus->Register(Plugin::OnSKSERegisterPapyrusFunctionsProxy)
			&&
				skse_messaging->RegisterListener(skse->GetPluginHandle(), "SKSE", Plugin::OnSKSEMessageProxy);
	}

	bool Plugin::OnSKSERegisterPapyrusFunctionsProxy(VMClassRegistry *registry)
	{
		if (registry == nullptr)
		{
			Plugin::Log(LOGL_VERBOSE, "(SKSE Register Functions Event) Plugin: SKSE passed a invalid argument. Ignoring.");

			return false;
		}

		for (IEventHandler *event_handler : GetInstance()->event_handler_set)
			if (event_handler) event_handler->OnSKSERegisterPapyrusFunctions(registry);

		return true;
	}

	void Plugin::OnSKSEMessageProxy(SKSEMessagingInterface::Message *message)
	{
		if (message == nullptr)
		{
			Log(LOGL_VERBOSE, "(SKSE Message Event) Plugin: SKSE passed a invalid argument. Ignoring.");

			return;
		}

		switch (message->type)
		{
			case SKSEMessagingInterface::kMessage_PostLoadGame:
			case SKSEMessagingInterface::kMessage_NewGame:
			{
				Log(LOGL_VERBOSE, "Plugin: Game is ready");
				GetInstance()->is_game_ready = true;
			} break;

			case SKSEMessagingInterface::kMessage_PreLoadGame:
			{
				Log(LOGL_VERBOSE, "Plugin: Game is not ready");
				GetInstance()->is_game_ready = false;
			} break;
		}

		for (IEventHandler *event_handler : GetInstance()->event_handler_set)
			if(event_handler) event_handler->OnSKSEMessage(message);
	}

	bool Plugin::IsGameReady()
	{
		return is_game_ready && is_plugin_active;
	}

	inline 
	void Plugin::Log(unsigned int level, const char *fmt, ...)
	{
		static std::mutex mtx;
		
		va_list args;
		std::string cfmt;
		
		if (level > DEBUG)
			return;

		va_start(args, fmt);

		switch(level)
		{
			case LOGL_CRITICAL: cfmt.assign("[CRITICAL] "); break;
			case LOGL_WARNING:	cfmt.assign("[ WARNING] "); break;
			case LOGL_INFO:		cfmt.assign("[    INFO] "); break;
			case LOGL_DETAILED: cfmt.assign("[      ->| "); break;
			case LOGL_VERBOSE:	cfmt.assign("[ VERBOSE] "); break;
		}
		
		cfmt.append(fmt);

		mtx.lock();
		gLog.Log(IDebugLog::kLevel_Message, cfmt.c_str(), args);
		mtx.unlock();

		va_end(args);
	}
}

extern "C"
{
	bool __declspec(dllexport) SKSEPlugin_Query(const SKSEInterface * skse, PluginInfo * info)
	{
		return SKU::Plugin::GetInstance()->OnSKSEQuery(skse, info);
	}

	bool __declspec(dllexport) SKSEPlugin_Load(const SKSEInterface * skse)
	{
		return SKU::Plugin::GetInstance()->OnSKSELoad(skse);
	}
}

extern "C"
{
	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
	{
		switch (fdwReason)
		{
			case DLL_PROCESS_ATTACH:
			{
				gLog.OpenRelative(CSIDL_MYDOCUMENTS, PLUGIN_RELATIVE_LOG_PATH);

				if (SKU::Plugin::GetInstance()->Initialize() == false)
					return FALSE;
			} break;

			case DLL_PROCESS_DETACH:
			{
				SKU::Plugin::GetInstance()->Stop();
			} break;
		}

		return TRUE;
	}
}
