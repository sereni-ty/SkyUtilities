#pragma once

#include "Net/RequestPool.h"

#include <memory>

namespace SKU::Net {
  class RequestManagerBase
  {
    public:
    RequestManagerBase();

    public:
    bool AddRequest(Request::Ptr request) noexcept;
    virtual void RemoveRequest(Request::Ptr request);

    Request::Ptr GetRequestByID(unsigned request_id) noexcept;

    protected:
    virtual void OnRequestAdded(Request::Ptr request)
    {}
    virtual void OnRequestRemoval(Request::Ptr request)
    {}

    public:
    virtual void Initialize() = 0;
    virtual void Cleanup() = 0;

    virtual void Stop() = 0;
    virtual void Start() = 0;

    protected:
    RequestPool pool;
  };
}
