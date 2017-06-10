#include "Net/Request.h"
#include "Plugin.h"

#include <exception>
#include <set>

namespace SKU::Net {
  static std::set<uint32_t> distributed_ids;

  static inline uint32_t GenerateRequestID(uint32_t requested_id = 0)
  {
    uint32_t id = 0;

    if (requested_id != 0)
    {
      if (distributed_ids.find(requested_id) == distributed_ids.end())
      {
        id = requested_id;
      }
      else
      {
        Plugin::Log(LOGL_WARNING, "Request: ID %d is already in use..", requested_id);
        goto on_request_fail;
      }
    }
    else
    {
    on_request_fail:
      if (distributed_ids.empty() == true)
      {
        id = 1;
      }
      else
      {
        id = 1 + *std::max_element(distributed_ids.begin(), distributed_ids.end());
      }
    }

    if (id != 0)
    {
      distributed_ids.emplace(id);
    }

    return id;
  }

  static inline void ReleaseRequestID(uint32_t id)
  {
    distributed_ids.erase(id);
  }

  Request::Request(uint32_t pre_set_id)
    : state(sWaitingForSetup)
  {
    id = GenerateRequestID(pre_set_id);
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

    ReleaseRequestID(id);
  }

  uint32_t Request::Request::GetID() noexcept
  {
    return id;
  }

  Request::State Request::GetState() noexcept
  {
    return state;
  }

  uint32_t Request::GetTimeout() noexcept
  {
    return timeout;
  }

  RequestEventHandler::Ptr Request::GetHandler()
  {
    return handler;
  }

  void Request::SetState(State state) noexcept
  {
    this->state = state;
  }

  void Request::SetTimeout(uint32_t ms) noexcept
  {
    timeout = ms;
  }

  void Request::SetHandler(RequestEventHandler::Ptr handler)
  {
    this->handler.swap(handler);
  }
}