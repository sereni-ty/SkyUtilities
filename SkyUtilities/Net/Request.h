#pragma once

#include "Lockable.h"

#include "Net/RequestProtocolContext.h"
#include "Net/RequestEventHandler.h"

#include <string>
#include <memory>
#include <mutex>

namespace SKU::Net {
  class Request : public Lockable
  {
    public:
    using Ptr = std::shared_ptr<Request>;

    public:
    enum State
    {
      sBlacklisted = -2,		// Blocked from adding requests indefinitely

      sFailed = 0,
      sOK = 1,

      sWaitingForSetup,
      sReady,
      sPending,
    };

    public:
    Request(uint32_t pre_set_id = 0);
    ~Request();

    public:
    template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
    static Ptr Create(uint32_t pre_set_id = 0);

    public:
    void Stop();

    void Cleanup();

    public:
    uint32_t GetID() noexcept;
    State GetState() noexcept;
    uint32_t GetTimeout() noexcept; // Timeout in ms
    RequestEventHandler::Ptr GetHandler();

    template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
    typename ProtocolContextType::Ptr GetProtocolContext();

    public:
    void SetState(State state) noexcept;
    void SetTimeout(uint32_t ms) noexcept;
    void SetHandler(RequestEventHandler::Ptr handler);

    private:
    uint32_t id;
    State state;
    IRequestProtocolContext::Ptr proto_ctx;
    unsigned timeout;

    RequestEventHandler::Ptr handler;

    std::mutex mtx;
  };

#	include "Request.inl"
}
