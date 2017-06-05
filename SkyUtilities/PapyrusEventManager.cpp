#include "PapyrusEventManager.h"

#include "Plugin.h"

namespace SKU {
  PapyrusEventManager::PapyrusEventManager()
    : pause(false)
  {}

  PapyrusEventManager::~PapyrusEventManager()
  {
    UnregisterAll();
    RemoveRecipients();
  }

  void PapyrusEventManager::Pause()
  {
    pause = true;
  }

  void PapyrusEventManager::Unpause()
  {
    if (pause == false)
    {
      return;
    }

    VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();

    for (auto &event_map_entry : event_map)
    {
      for (auto &event : event_map.at(event_map_entry.first))
      {
        if (event->queued > 0)
        {
          continue;
        }

        auto &event_key = event_key_map.at(event_map_entry.first);

        for (PapyrusEventRecipient recipient : recipient_map.at(event_map_entry.first))
        {
          registry->QueueEvent(recipient, &event_key, event.get());
          event->queued++;
        }
      }
    }

    pause = false;
  }

  bool PapyrusEventManager::Send(const std::string &event_name, PapyrusEvent::Args &&args)
  {
    VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();

    if (event_key_map.find(event_name) == event_key_map.end())
    {
      return false;
    }

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Sending event '%s' with %d arguments.",
      event_name.c_str(), args.size());

    auto event = std::make_unique<PapyrusEvent>(std::move(args));
    auto &event_key = event_key_map.at(event_name);

    if (pause == false)
    {
      for (PapyrusEventRecipient recipient : recipient_map.at(event_name))
      {
        registry->QueueEvent(recipient, &event_key, event.get());
        event->queued++;
      }
    }

    event_map.at(event_name).emplace(std::move(event));

    //Cleanup();

    return true;
  }

  void PapyrusEventManager::Register(const std::string &event_name)
  {
    try
    {
      event_key_map.try_emplace(event_name, BSFixedString(event_name.c_str()));

      recipient_map.try_emplace(event_name, std::unordered_set<PapyrusEventRecipient>());
      event_map.try_emplace(event_name, std::unordered_set< std::unique_ptr<PapyrusEvent> >());

      Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Registered event '%s'.", event_name.c_str());
    }
    catch (std::exception)
    {
      Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Exception: Event '%s' not registered.", event_name.c_str());
    }
  }

  void PapyrusEventManager::Unregister(const std::string &event_name)
  {
    if (event_key_map.find(event_name) == event_key_map.end())
    {
      return;
    }

    if (event_map.at(event_name).size() >= 0)
    {
      // TODO: Right now this function only gets called if the plugin is quitting.
      //       If that is not the case anymore and there are events queued up, then
      //       this function should be implemented properly!
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Unregistering event '%s'.", event_name.c_str());

    event_key_map.erase(event_name);
  }

  void PapyrusEventManager::UnregisterAll()
  {
    event_key_map.clear();
  }

  bool PapyrusEventManager::Remaining(const std::string &event_name)
  {
    if (event_map.find(event_name) == event_map.end())
    {
      return false;
    }

    return event_map.at(event_name).size() > 0;
  }

  void PapyrusEventManager::Cleanup()
  {
    Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Cleaning up..");

    for (auto &event_map_entry : event_map)
    {
      if (event_map.at(event_map_entry.first).empty() == true)
      {
        continue;
      }

      for (auto &event : event_map.at(event_map_entry.first))
      {
        if (event->queued == 0 || event->queued != event->sent)
        {
          continue;
        }

        event_map.at(event_map_entry.first).erase(event);
      }
    }

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Cleaned up.");
  }

  bool PapyrusEventManager::AddRecipient(const std::string &event_name, TESForm *recipient)
  {// TODO: Serialize policy handle?
    VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();
    IObjectHandlePolicy *policy = registry->GetHandlePolicy();

    static PapyrusEventRecipient invalid_handle = policy->GetInvalidHandle();
    PapyrusEventRecipient recipient_handle;

    if (event_key_map.find(event_name) == event_key_map.end())
    {
      return false;
    }

    recipient_handle = policy->Create(recipient->formType, recipient);

    if (recipient_handle == invalid_handle)
    {
      return false;
    }

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Adding recipient '%s' to event '%s'.",
      recipient->GetFullName(), event_name.c_str());

    recipient_map.at(event_name).emplace(recipient_handle);

    return true;
  }

  //void RemoveRecipient(const std::string &event_name, TESForm *recpient);
  //void RemoveRecipientEntirely(TESForm *recpient);
  void PapyrusEventManager::RemoveRecipients()
  {
    VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();
    IObjectHandlePolicy *policy = registry->GetHandlePolicy();

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Removing every recipient for any event.");

    for (auto &event_recipients : recipient_map)
    {
      for (auto &event_recipient : event_recipients.second)
      {
        policy->Release(event_recipient);
      }
    }

    recipient_map.clear();
  }

  void PapyrusEventManager::Serialize(std::stack<ISerializeable::SerializationEntity> &serialized_entities)
  {
    SerializationEntity serialized;

    std::get<ISerializeable::idType>(serialized) = PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_TYPE;
    std::get<ISerializeable::idVersion>(serialized) = PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_VERSION;
    std::get<ISerializeable::idStream>(serialized) = std::stringstream();

    SerializeIntegral(serialized, recipient_map.size()); // event amount

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Saving %d registered events",
      recipient_map.size());

    for (auto &recipient_map_entry : recipient_map)
    {
      SerializeString(serialized, recipient_map_entry.first);
      SerializeIntegral(serialized, recipient_map_entry.second.size()); // event recipient amount

      for (PapyrusEventRecipient recipient : recipient_map_entry.second)
      {
        SerializeIntegral(serialized, recipient); // recipient
      }
    }

    serialized_entities.push(std::move(serialized));
  }

  void PapyrusEventManager::Deserialize(ISerializeable::SerializationEntity &serialized)
  {
    uint32_t event_amount = 0, recipient_amount;
    std::string event_name;

    if (IsRequestedSerialization(serialized) == false)
    {
      return;
    }

    if (std::get<ISerializeable::idStream>(serialized).tellg() == std::streampos(0))
    {
      Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: No events were registered.");
      return;
    }

    DeserializeIntegral(serialized, event_amount);

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Loading %d registrered events from save.",
      event_amount);

    for (uint32_t i = 0; i < event_amount; i++)
    {
      DeserializeString(serialized, event_name);
      DeserializeIntegral(serialized, recipient_amount);

      Plugin::Log(LOGL_DETAILED, "PapyrusEventManager: Setting up event '%d' with %d recipients",
        event_name.c_str(), recipient_amount);

      if (recipient_amount > 0)
      {
        Register(event_name);

        for (int j = 0; j < recipient_amount; j++)
        {
          PapyrusEventRecipient recipient;
          DeserializeIntegral(serialized, recipient);

          recipient_map.at(&event_name[0]).emplace(recipient);
        }
      }
    }

    if (std::get<ISerializeable::idStream>(serialized).fail() == true)
    {
      Plugin::Log(LOGL_WARNING, "PapyrusEventManager: Error occurred while loading.");
    }
  }

  bool PapyrusEventManager::IsRequestedSerialization(ISerializeable::SerializationEntity &serialized)
  {
    if (std::get<ISerializeable::idType>(serialized) != PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_TYPE)
    {
      return false;
    }

    if (std::get<ISerializeable::idVersion>(serialized) != PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_VERSION)
    {
      Plugin::Log(LOGL_VERBOSE, "PapyrusEventManager: Unsupported data version.");
      return false;
    }

    return true;
  }
}