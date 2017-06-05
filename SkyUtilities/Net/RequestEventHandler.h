#pragma once

#include <memory>

#define REQUEST_EVENT_HANDLER_DEF(type_id)\
public:\
const static uint32_t TypeID = type_id;\
inline uint32_t GetTypeID(){ return type_id; }

namespace SKU::Net {
  class Request;

  class RequestEventHandler // TODO: implement request into class
  {
    REQUEST_EVENT_HANDLER_DEF(0)

    public:
    using Ptr = std::shared_ptr<RequestEventHandler>;

    public:
    virtual void OnRequestFinished(std::shared_ptr<Request> request)
    {}
  };
}
