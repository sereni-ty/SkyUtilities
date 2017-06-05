#pragma once

#include "Serializeable.h"
#include "PapyrusEvent.h"

#include <skse/PapyrusVM.h>
#include <skse/PapyrusObjects.h>
#include <skse/PapyrusEvents.h>

#include <unordered_map>
#include <unordered_set>

#include <vector>
#include <any>
#include <string>
#include <memory>

#define PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_TYPE MACRO_SWAP32('PESU')
#define PLUGIN_PAPYRUS_EVENTS_SERIALIZATION_VERSION 1

namespace SKU {
  using PapyrusEventRecipient = uint64_t;

  class PapyrusEventManager :public ISerializeable
  {
    friend bool PapyrusEvent::Copy(Output * dst);

    public:
    using Ptr = std::unique_ptr<PapyrusEventManager>;

    public:
    PapyrusEventManager();
    ~PapyrusEventManager();

    public:
    void Pause();
    void Unpause();

    public:
    bool Send(const std::string &event_name, PapyrusEvent::Args &&args);

    public:
    void Register(const std::string &event_name);
    void Unregister(const std::string &event_name);
    void UnregisterAll();

    public:
    bool Remaining(const std::string &event_name);

    protected:
    void Cleanup();

    public:
    bool AddRecipient(const std::string &event_name, TESForm *recipient);

    //void RemoveRecipient(const std::string &event_name, TESForm *recipient);
    //void RemoveRecipientEntirely(TESForm *recipient);
    void RemoveRecipients();

    // ISerializeable
    //
    public:
    virtual void Serialize(std::stack<ISerializeable::SerializationEntity> &serialized_entities) final;
    virtual void Deserialize(ISerializeable::SerializationEntity &serialized) final;
    virtual bool IsRequestedSerialization(ISerializeable::SerializationEntity &serialized) final;

    private:
    using RecipientMap = std::unordered_map</* event name: */std::string, std::unordered_set< PapyrusEventRecipient > >;
    using EventMap = std::unordered_map </* event name: */std::string, std::unordered_set< std::unique_ptr<PapyrusEvent> > >;
    using EventKeyMap = std::unordered_map < std::string, BSFixedString >;

    RecipientMap recipient_map;
    EventMap event_map;
    EventKeyMap event_key_map;

    bool pause;
  };
}
