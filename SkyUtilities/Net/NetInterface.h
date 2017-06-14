#pragma once

#include "Events.h"
#include "Serializeable.h"
#include "Configuration.h"

#include "Net/HTTP/RequestManager.h"
#include "Net/HTTP/RequestProtocolContext.h"

#include <memory>

struct StaticFunctionTag;
class TESForm;

namespace SKU::Net {
  namespace Config {
    extern Configuration::Setting<uint32_t> PapyrusCallsPerTimeFrame;
    extern Configuration::Setting<uint32_t> PapyrusMaxTransgressions;
    extern Configuration::Setting<uint32_t> PapyrusTimeFrame;
    extern Configuration::Setting<long> RequestsDefaultTimeout;
  }

  class Interface : public IEventHandler
  {
    friend class Plugin;

    public:
    using Ptr = std::unique_ptr<Interface>;

    public:
    Interface();
    ~Interface();

    public:
    void Start();
    void Stop(); // Stop every net activity

    public:
    HTTP::RequestManager::Ptr &GetHTTPRequestManager();

    public:
    static long HTTPGETRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, long timeout);
    static long HTTPPOSTRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, BSFixedString body, long timeout);

    static long GetNexusModInfo(StaticFunctionTag*, TESForm *form, BSFixedString mod_id);
    static long GetLLabModInfo(StaticFunctionTag*, TESForm *form, BSFixedString mod_id);

    static long HTTPRequest(uint32_t request_handler_type_id, TESForm *form, HTTP::RequestProtocolContext::Method method, std::string url, std::string body, long timeout);

    static BSFixedString URLEncode(StaticFunctionTag*, BSFixedString raw);
    static BSFixedString URLDecode(StaticFunctionTag*, BSFixedString encoded);

    // IEventHandler
    //
    public:
    virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept final;

    // ISerializeable
    //
    public:
    virtual void Serialize(std::stack<ISerializeable::SerializationEntity> &serialized_entities) final;
    virtual void Deserialize(ISerializeable::SerializationEntity &serialized) final;
    virtual bool IsRequestedSerialization(const ISerializeable::SerializationEntity &serialized) final;

    public:
    enum PapyrusEvent
    {
      evHTTPRequestFinished,
      evModInfoRetrieval
    };

    static std::string GetEventString(PapyrusEvent event) noexcept;

    private:
    HTTP::RequestManager::Ptr http_requestmanager;
  };
}
