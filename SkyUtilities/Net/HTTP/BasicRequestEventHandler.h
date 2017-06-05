#pragma once

#include "Net/Request.h"
#include "Net/RequestEventHandler.h"

namespace SKU::Net::HTTP {
  class BasicRequestEventHandler : public RequestEventHandler
  {
    REQUEST_EVENT_HANDLER_DEF('BREH')

    public:
    using Ptr = std::shared_ptr<BasicRequestEventHandler>;

    public:
    virtual void OnRequestFinished(Request::Ptr request) final;
  };
}
