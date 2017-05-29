#include "PapyrusEventHandler.h"

#include "Plugin.h"

namespace SKU {
  PapyrusEventHandler::PapyrusEventHandler()
  {}

  PapyrusEventHandler::~PapyrusEventHandler()
  {
    UnregisterAll();
    RemoveRecipients();
  }

  bool PapyrusEventHandler::Send(const std::string &event_name, PapyrusEvent::Args &&args)
  {
    VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();

    if (event_key_map.find(event_name) == event_key_map.end())
      return false;

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Sending event '%s' with %d arguments.",
      event_name.c_str(), args.size());

    auto event = std::make_unique<PapyrusEvent>(std::move(args));
    auto &event_key = event_key_map.at(event_name);

    for (PapyrusEventRecipient recipient : recipient_map.at(event_name))
      registry->QueueEvent(recipient, &BSFixedString(event_name.c_str()), event.get());

    event_map.at(event_name).emplace(std::move(event));

    return true;
  }

  void PapyrusEventHandler::Register(const std::string &event_name)
  {
    try
    {
      event_key_map.try_emplace(event_name, BSFixedString(event_name.c_str()));

      recipient_map.try_emplace(event_name, std::unordered_set<PapyrusEventRecipient>());
      event_map.try_emplace(event_name, std::unordered_set< std::unique_ptr<PapyrusEvent> >());

      Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Registered event '%s'.", event_name.c_str());
    }
    catch (std::exception)
    {
      Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Exception: Event '%s' not registered.", event_name.c_str());
    }
  }

  void PapyrusEventHandler::Unregister(const std::string &event_name)
  {
    if (event_key_map.find(event_name) == event_key_map.end())
      return;

    if (event_map.at(event_name).size() >= 0)
    {
      // TODO: Right now this function only gets called if the plugin is quitting.
      //       If that is not the case anymore and there are events queued up, then
      //       this function should be implemented properly!
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Unregistering event '%s'.", event_name.c_str());

    event_key_map.erase(event_name);
  }

  void PapyrusEventHandler::UnregisterAll()
  {
    event_key_map.clear();
  }

  void PapyrusEventHandler::Cleanup(const PapyrusEvent *event)
  {
    for (auto &event_type_set : event_map)
    {
      for (auto &actual_event : event_type_set.second)
      {
        if (actual_event.get() == event)
        {
          Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Removed processed event.");
          event_type_set.second.erase(actual_event);
        }
      }
    }
  }

  bool PapyrusEventHandler::AddRecipient(const std::string &event_name, TESForm *recipient)
  {// TODO: Serialize policy handle?
    VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();
    IObjectHandlePolicy *policy = registry->GetHandlePolicy();

    static PapyrusEventRecipient invalid_handle = policy->GetInvalidHandle();
    PapyrusEventRecipient recipient_handle;

    if (event_key_map.find(event_name) == event_key_map.end())
      return false;

    recipient_handle = policy->Create(recipient->formType, recipient);

    if (recipient_handle == invalid_handle)
      return false;

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Adding recipient '%s' to event '%s'.",
      recipient->GetFullName(), event_name.c_str());

    recipient_map.at(event_name).emplace(recipient_handle);

    return true;
  }

  //void RemoveRecipient(const std::string &event_name, TESForm *recpient);
  //void RemoveRecipientEntirely(TESForm *recpient);
  void PapyrusEventHandler::RemoveRecipients()
  {
    VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();
    IObjectHandlePolicy *policy = registry->GetHandlePolicy();

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Removing every recipient for any event.");

    for (auto &event_recipients : recipient_map)
      for (auto &event_recipient : event_recipients.second)
        policy->Release(event_recipient);

    recipient_map.clear();
  }

  void PapyrusEventHandler::OnSKSESaveGame(SKSESerializationInterface *serilization_interface)
  {
    Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Saving events and recipients.");

    bool write_fail = false;
    uint32_t tmp;

    if (serilization_interface->OpenRecord(PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_TYPE, PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_VERSION) == false)
    {
      Plugin::Log(LOGL_WARNING, "PapyrusEventHandler: Couldn't open record to save data.");
      return;
    }

    if (serilization_interface->WriteRecordData(&(tmp = recipient_map.size()), 4) == false) // events
    {
      Plugin::Log(LOGL_WARNING, "PapyrusEventHandler: Unable to save data.");
      return;
    }

    for (auto &recipient_map_entry : recipient_map)
    {
      Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Writing event entry..");
      write_fail = true;

      FAIL_BREAK_WRITE(serilization_interface, &(tmp = recipient_map_entry.first.length()), 4); // event name size
      FAIL_BREAK_WRITE(serilization_interface, recipient_map_entry.first.c_str(), tmp); // event name
      FAIL_BREAK_WRITE(serilization_interface, &(tmp = recipient_map_entry.second.size()), 4); // event recipients

      write_fail = false;

      for (PapyrusEventRecipient recipient : recipient_map_entry.second)
      {
        Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Writing event (%s) recipient entry (%016Xh)..", recipient_map_entry.first, recipient);
        write_fail = true;
        FAIL_BREAK_WRITE(serilization_interface, &recipient, 8); // event recipient
        write_fail = false;
      }

      if (write_fail == true)
        break;
    }

    if (write_fail == true)
    {
      Plugin::Log(LOGL_WARNING, "PapyrusEventHandler: Failed to write save data.");
    }
  }

  void PapyrusEventHandler::OnSKSELoadGame(SKSESerializationInterface *serilization_interface, SInt32 type, SInt32 version, SInt32 length)
  {
    if (type != PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_TYPE)
    {
      return;
    }

    if (version != PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_VERSION)
    {
      Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Unsupported data version.");
      return;
    }

    if (length == 0)
    {
      Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: No events were registered.");
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Loading event recipients.");

    uint32_t events = 0, tmp;
    bool read_fail = false;

    if (serilization_interface->ReadRecordData(&events, 4) == 0) // amount of events
    {
      Plugin::Log(LOGL_WARNING, "PapyrusEventHandler: Unable to load data from save.");
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: %d event types loaded..", events);

    for (int i = 0; i < events; i++)
    {
      std::vector<char> buf;

      read_fail = true;

      FAIL_BREAK_READ(serilization_interface, &tmp, 4); // event name size

      try
      {
        buf.resize(tmp + 1, 0);
      }
      catch (std::exception)
      {
        Plugin::Log(LOGL_WARNING, "PapyrusEventHandler: Failed to resize buffer to read event name. Skipping Loading process.");
        break;
      }

      FAIL_BREAK_READ(serilization_interface, &buf[0], tmp); // event name
      Register(&buf[0]);

      FAIL_BREAK_READ(serilization_interface, &tmp, 4); // amount of recipients

      Plugin::Log(LOGL_VERBOSE, "PapyrusEventHandler: Loading %d recipients for event %s", tmp, &buf[0]);

      read_fail = false;

      for (int j = 0; j < tmp; j++)
      {
        PapyrusEventRecipient recipient, new_handle;
        FAIL_BREAK_READ(serilization_interface, &recipient, 8);

        if (serilization_interface->ResolveHandle(recipient, &new_handle) == false)
        {
          Plugin::Log(LOGL_WARNING, "PapyrusEventHandler: Failed to resolve new recipient handle.");
          break;
        }

        recipient_map.at(&buf[0]).emplace(new_handle);
      }

      if (read_fail == true)
        break;
    }

    if (read_fail == true)
    {
      Plugin::Log(LOGL_WARNING, "PapyrusEventHandler: Failed to read save data.");
    }
  }
}