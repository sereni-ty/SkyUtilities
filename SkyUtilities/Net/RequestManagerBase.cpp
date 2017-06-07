#include "Net/RequestManagerBase.h"

#include "Plugin.h"

namespace SKU::Net {
  RequestManagerBase::RequestManagerBase()
    : pool()
  {}

  bool RequestManagerBase::AddRequest(Request::Ptr request) noexcept
  {
    try
    {
      if (pool.AddRequest(request).second == true)
      {
        OnRequestAdded(request);
        return true;
      }
    }
    catch (std::exception e)
    {
    }

    return false;
  }

  void RequestManagerBase::RemoveRequest(Request::Ptr request)
  {
    OnRequestRemoval(request);

    try
    {
      pool.Get().erase(request);
    }
    catch (std::exception)
    {
      Plugin::Log(LOGL_WARNING, "RequestManagerBase: Failed to remove request..");
    }
  }

  Request::Ptr RequestManagerBase::GetRequestByID(unsigned request_id) noexcept
  {
    try
    {
      return pool.GetRequestByID(request_id);
    }
    catch (std::exception)
    {
    }

    return nullptr;
  }
}