#pragma once

#include <memory>

namespace SKU::Net {
  class Request;

  class RequestEventHandler
  {
    public:
    using Ptr = std::shared_ptr<RequestEventHandler>;

    public:
    virtual void OnRequestFinished(std::shared_ptr<Request> request)
    {}

    public:
    static const uint32_t TypeID = 0; // TODO: I know, I know..
  };
}
