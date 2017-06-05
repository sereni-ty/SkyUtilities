#pragma once
#include "Net/Request.h"
#include "Net/RequestEventHandler.h"

namespace SKU::Net::HTTP { // TODO: Add failure boolean value to event
  class NexusModInfoRequestEventHandler : public RequestEventHandler
  {
    REQUEST_EVENT_HANDLER_DEF('NMRH')

    public:
    using Ptr = std::shared_ptr<NexusModInfoRequestEventHandler>;

    public:
    virtual void OnRequestFinished(Request::Ptr request) final;
  };

  class LLabModInfoRequestEventHandler : public RequestEventHandler
  {
    REQUEST_EVENT_HANDLER_DEF('LLRH')

    public:
    using Ptr = std::shared_ptr<LLabModInfoRequestEventHandler>;

    public:
    virtual void OnRequestFinished(Request::Ptr request) final;
  };
}
