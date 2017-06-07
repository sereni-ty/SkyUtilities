#pragma once

#include <memory>

#define REQUEST_EVENT_HANDLER_DEF(type_id)\
public:\
const static uint32_t TypeID = type_id;\
virtual inline uint32_t GetTypeID() final { return type_id; }

namespace SKU::Net {
  class Request;

  class RequestEventHandler // TODO: implement request into class
  {
    public:
    using Ptr = std::shared_ptr<RequestEventHandler>;

    public:
    virtual void OnRequestFinished(std::shared_ptr<Request> request)
    {}

    public:
    const static uint32_t TypeID = 0;

    virtual inline uint32_t GetTypeID()
    {
      return 0;
    }
  };
}
