#include "Net/Request.h"
#include "Plugin.h"

#include <exception>

namespace SKU::Net {
  static unsigned GLOBAL_REQUEST_ID_COUNTER = 0;

  Request::Request(int pre_set_id) noexcept
    : state(sWaitingForSetup)
  {
    if (pre_set_id > 0) // TODO: Temporary solution. Get ID from a distributor who knows exactly which ids are free to use
    {
      id = pre_set_id;
      GLOBAL_REQUEST_ID_COUNTER = pre_set_id + 100;
    }
    else
    {
      id = ++GLOBAL_REQUEST_ID_COUNTER;
    }
  }

  Request::~Request()
  {
    Stop();
  }

  void Request::Stop()
  {}

  void Request::Cleanup()
  {
    if (proto_ctx == nullptr)
    {
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "Request (id: %d): Cleaning up request.", id);

    proto_ctx->Cleanup();
  }

  int Request::Request::GetID() noexcept
  {
    return id;
  }

  Request::State Request::GetState() noexcept
  {
    return state;
  }

  unsigned Request::GetTimeout() noexcept
  {
    return timeout;
  }

  RequestEventHandler::Ptr Request::GetHandler()
  {
    return handler;
  }

  void Request::SetID(int id) noexcept // TODO: check if it's still used
  {
    if (GLOBAL_REQUEST_ID_COUNTER < id)
      GLOBAL_REQUEST_ID_COUNTER = id + 10;

    this->id = id;
  }

  void Request::SetState(State state) noexcept
  {
    this->state = state;
  }

  void Request::SetTimeout(unsigned ms) noexcept
  {
    timeout = ms;
  }

  void Request::SetHandler(RequestEventHandler::Ptr handler)
  {
    this->handler.swap(handler);
  }
}