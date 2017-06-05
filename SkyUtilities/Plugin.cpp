#include "Plugin.h"

#include "Serializeable.h"

#include "PapyrusEventManager.h"

#include "Net/NetInterface.h"
#include "Net/HTTP/RequestManager.h"

#include <common/IDebugLog.h>
#include <skse/PluginAPI.h>
#include <skse/skse_version.h>
#include <skse/PapyrusArgs.h>

#ifdef DEBUG
# include <debugapi.h>
#endif

#include <shlobj.h>

#include <mutex>
#include <list>

// ==== CRITICAL
// TODO: investigations have to be made.. whenever a request ist saved (thus not completely processed or in the midst of it) the save is not salvageable. find out what exactly causes this behaivor.
// ====
// TODO: create test classes to test if TEST macro is set just after "game ready" has been set
// TODO: might need a configuration file (net: requests per seconds, max response size)

namespace SKU {
  Plugin::Plugin()
    : is_game_ready(false), is_plugin_ready(false)
  {
    Log(LOGL_INFO, "%s (%s)\n", PLUGIN_NAME, PLUGIN_RELEASE_VERSION_STR);

#ifdef DEBUG
    Log(LOGL_INFO, "Waiting for Debugger to attach..");

    while (IsDebuggerPresent() == FALSE)
    {
      Sleep(100);
    }

    Log(LOGL_INFO, "Debugger attached.");
#endif
  }

  Plugin::~Plugin()
  {}

  void Plugin::Stop()
  {
    is_plugin_ready = false;
  }

  bool Plugin::OnSKSEQuery(const SKSEInterface *skse, PluginInfo *info)
  {
    if (skse == nullptr || info == nullptr)
    {
      return false;
    }

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
    {
      return false;
    }

    SKSEPapyrusInterface *skse_papyrus = reinterpret_cast<SKSEPapyrusInterface*>(skse->QueryInterface(kInterface_Papyrus));
    SKSEMessagingInterface *skse_messaging = reinterpret_cast<SKSEMessagingInterface*>(skse->QueryInterface(kInterface_Messaging));
    SKSESerializationInterface *skse_serialization = reinterpret_cast<SKSESerializationInterface*>(skse->QueryInterface(kInterface_Serialization));

    if (skse_papyrus == nullptr || skse_messaging == nullptr || skse_serialization == nullptr)
    {
      Log(LOGL_INFO, "SKSE failed to return valid interfaces. Stopping plugin.");
      Log(LOGL_DETAILED, "Interfaces:\n-\tPapyrus Interface: %s\n-\tMessaging: %s\n-\tSerialization: %s",
        (skse_papyrus ? "valid" : "invalid"),
        (skse_messaging ? "valid" : "invalid"),
        (skse_serialization ? "valid" : "invalid"));

      return false;
    }

    skse_serialization->SetUniqueID(skse->GetPluginHandle(), PLUGIN_SERIALIZATION_UID);
    skse_serialization->SetSaveCallback(skse->GetPluginHandle(), OnSKSESaveGameProxy);
    skse_serialization->SetLoadCallback(skse->GetPluginHandle(), OnSKSELoadGameProxy);

    net = std::make_unique<Net::Interface>();
    papyrus_event_manager = std::make_unique<PapyrusEventManager>();

    is_plugin_ready = skse_papyrus->Register(Plugin::OnSKSERegisterPapyrusFunctionsProxy)
      && skse_messaging->RegisterListener(skse->GetPluginHandle(), "SKSE", Plugin::OnSKSEMessageProxy)
      && papyrus_event_manager != nullptr
      && net != nullptr;

    return is_plugin_ready;
  }

  bool Plugin::OnSKSERegisterPapyrusFunctionsProxy(VMClassRegistry *registry)
  {
    if (registry == nullptr)
    {
      Plugin::Log(LOGL_VERBOSE, "(SKSE Register Functions Event) Plugin: SKSE passed a invalid argument. Ignoring.");

      return false;
    }

    GetInstance()->net->OnSKSERegisterPapyrusFunctions(registry);

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
      case SKSEMessagingInterface::kMessage_NewGame:
      {
        GetInstance()->OnNewGame();
      } break;

      case SKSEMessagingInterface::kMessage_PreLoadGame:
      {
        GetInstance()->OnPreLoadGame();
      } break;

      case SKSEMessagingInterface::kMessage_PostLoadGame:
      {
        GetInstance()->OnPostLoadGame();
      } break;

      case SKSEMessagingInterface::kMessage_SaveGame:
      {
        GetInstance()->OnSaveGame();
      } break;
    }
  }

  void Plugin::OnSKSESaveGameProxy(SKSESerializationInterface *serialization_interface)
  {
    std::stack<ISerializeable::SerializationEntity> serialized_entities;

    Log(LOGL_INFO, "Plugin: Saving.");

    if (serialization_interface == nullptr)
    {
      Log(LOGL_WARNING, "Plugin: Invalid serialization interface pointer. Unable to save game.");
      return;
    }

    GetInstance()->papyrus_event_manager->Serialize(serialized_entities);
    GetInstance()->net->Serialize(serialized_entities);

    while (serialized_entities.empty() == false)
    {
      ISerializeable::SerializationEntity serialized = std::move(serialized_entities.top());
      serialized_entities.pop();

      if (serialization_interface->OpenRecord(std::get<ISerializeable::idType>(serialized), std::get<ISerializeable::idVersion>(serialized)) == false
        || serialization_interface->WriteRecordData(std::get<ISerializeable::idStream>(serialized).str().c_str(), std::get<ISerializeable::idStream>(serialized).str().size()) == false)
      {
        Log(LOGL_WARNING, "Plugin: Serialization of %.*s failed (unable to open record). Skipping.",
          4, reinterpret_cast<char *>(&std::get<ISerializeable::idType>(serialized)));
      }
      else
      {
        Log(LOGL_VERBOSE, "Plugin: Serialization of %.*s succeeded.",
          4, reinterpret_cast<char *>(&std::get<ISerializeable::idType>(serialized)));
      }
    }

    Log(LOGL_INFO, "Plugin: Saved.");

    GetInstance()->papyrus_event_manager->Unpause();
  }

  void Plugin::OnSKSELoadGameProxy(SKSESerializationInterface *serialization_interface)
  {
    Log(LOGL_INFO, "Plugin: Loading.");

    if (serialization_interface == nullptr)
    {
      Log(LOGL_WARNING, "Plugin: Invalid serialization interface pointer. Unable to load game.");
      return;
    }

    UInt32 type, version, length;

    while (serialization_interface->GetNextRecordInfo(&type, &version, &length) == true)
    {
      ISerializeable::SerializationEntity serialized;
      std::stringstream streamed_data;
      std::vector<char> data;

      std::get<ISerializeable::idType>(serialized) = type;
      std::get<ISerializeable::idVersion>(serialized) = version;

      if (GetInstance()->papyrus_event_manager->IsRequestedSerialization(serialized) == false
        && GetInstance()->net->IsRequestedSerialization(serialized) == false)
      {
        continue;
      }

      data.resize(length);
      uint32_t length_read = serialization_interface->ReadRecordData(&data[0], length);

      if (length_read != length)
      {
        Plugin::Log(LOGL_WARNING, "Plugin: Reading serialized data of record (type: %.*s) failed (size: %d, read: %d). Skipping.",
          4, reinterpret_cast<char*>(&type), length, length_read);

        continue;
      }

      streamed_data.write(&data[0], length);
      data.clear();

      std::get<ISerializeable::idStream>(serialized) = std::move(streamed_data);

      Plugin::Log(LOGL_VERBOSE, "Plugin: Deserialization (record: %.*s)..", 4, reinterpret_cast<char *>(&type));

      GetInstance()->papyrus_event_manager->Deserialize(serialized);
      GetInstance()->net->Deserialize(serialized);
    }

    Log(LOGL_INFO, "Plugin: Loaded.");
  }

  void Plugin::OnNewGame()
  {
    is_game_ready = true;

    if (IsActive() == false)
    {
      return;
    }

    net->Start();
  }

  void Plugin::OnSaveGame()
  {
    papyrus_event_manager->Pause(); // unpause called in serialization.
  }

  void Plugin::OnPreLoadGame()
  {
    if (is_game_ready == true)
    {
      papyrus_event_manager->RemoveRecipients();
      papyrus_event_manager->UnregisterAll();

      net->Stop();
    }

    is_game_ready = false;
  }

  void Plugin::OnPostLoadGame()
  {
    is_game_ready = true;

    if (IsActive() == false)
    {
      return;
    }

    net->Start();
  }

  bool Plugin::IsActive()
  {
    return is_game_ready && is_plugin_ready;
  }

  Net::Interface::Ptr &Plugin::GetNetInterface()
  {
    return net;
  }

  PapyrusEventManager::Ptr &Plugin::GetPapyrusEventManager()
  {
    return papyrus_event_manager;
  }

  inline
    void Plugin::Log(unsigned int level, const char *fmt, ...)
  {
    static std::mutex mtx;

    va_list args;
    std::string cfmt;

    /*if (level > DEBUG)
      return;*/

    va_start(args, fmt);

    switch (level)
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
      } break;

      case DLL_PROCESS_DETACH:
      {
        SKU::Plugin::GetInstance()->Stop();
      } break;
    }

    return TRUE;
  }
}