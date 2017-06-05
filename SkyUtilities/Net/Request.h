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
    Request(int pre_set_id = -1) noexcept;
    ~Request();

    public:
    template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
    static Ptr Create(int pre_set_id = -1);

    public:
    void Stop();

    void Cleanup();

    public:
    int GetID() noexcept;
    State GetState() noexcept;
    unsigned GetTimeout() noexcept; // Timeout in ms
    RequestEventHandler::Ptr GetHandler();

    template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
    typename ProtocolContextType::Ptr GetProtocolContext();

    public:
    void SetID(int id) noexcept;
    void SetState(State state) noexcept;
    void SetTimeout(unsigned ms) noexcept;
    void SetHandler(RequestEventHandler::Ptr handler);

    private:
    int id;
    State state;
    IRequestProtocolContext::Ptr proto_ctx;
    unsigned timeout;

    RequestEventHandler::Ptr handler;

    std::mutex mtx;
  };

#	include "Request.inl"
}
