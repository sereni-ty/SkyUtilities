#pragma once

#include "Serializeable.h"
#include "Configuration.h"

#include "Net/RequestManagerBase.h"

#include <thread>
#include <future>
#include <memory>

#include <curl/curl.h>

#define PLUGIN_REQUEST_MANAGER_SERIALIZATION_TYPE MACRO_SWAP32('RMSU')
#define PLUGIN_REQUEST_MANAGER_SERIALIZATION_VERSION 1

namespace SKU::Config {
  extern Configuration::Setting<uint32_t> HTTPResponseSizeLimit;
}

namespace SKU::Net::HTTP { // TODO: request accessor class instead of lock.. move ptr around and back on "release" <-- ctx only accessible through that one
  class RequestManager : public SKU::Net::RequestManagerBase, public ISerializeable
  {
    public:
    using Ptr = std::unique_ptr<RequestManager>;

    public:
    RequestManager();
    ~RequestManager();

    private:
    Request::Ptr GetRequestByHandle(CURL *curl_handle);

    // RequestManagerBase
    //
    public:
    virtual void Initialize() override;
    virtual void Cleanup() override;

    virtual void Stop() override;
    virtual void Start() override;

    public:
    static void Process();

    private:
    bool SetupRequests();
    bool PerformRequests();
    bool CheckPendingRequests();

    // RequestManagerBase
    //
    public:
    virtual void OnRequestAdded(Request::Ptr request) override;
    virtual void OnRequestRemoval(Request::Ptr request) override;

    public:
    static size_t OnRequestResponse(char* data, size_t size, size_t nmemb, void *request_id);

    // ISerializeable
    //
    public:
    virtual void Serialize(std::stack<ISerializeable::SerializationEntity> &serialized_entities) final;
    virtual void Deserialize(ISerializeable::SerializationEntity &serialized) final;
    virtual bool IsRequestedSerialization(const ISerializeable::SerializationEntity &serialized) final;

    private:
    bool should_run;
    std::future<void> processing_thread;

    CURLM *curl_handle;
    CURLMcode curl_last_error;
  };
}
