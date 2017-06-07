#include "Net/RequestPool.h"

#include "Plugin.h"

#include <algorithm>

namespace SKU::Net {
  RequestPool::RequestPool()
    : pool()
  {}

  Request::Ptr RequestPool::GetRequestByID(int id)
  {
    auto& request_pos = std::find_if(pool.begin(), pool.end(), [&] (Request::Ptr request) -> bool
    {
      if (request->GetID() == id)
        return true;

      return false;
    });

    if (request_pos == pool.end())
      return nullptr;

    return *request_pos;
  }

  std::vector<Request::Ptr> RequestPool::GetRequestsByState(Request::State state)
  {
    std::vector<Request::Ptr> requests;

    for (Request::Ptr request : pool)
      if (request->GetState() == state)
        requests.push_back(request);

    return requests;
  }

  int RequestPool::GetCountByState(Request::State state)
  {
    return std::count_if(pool.begin(), pool.end(), [&] (Request::Ptr request) -> bool
    {
      return state == request->GetState();
    });
  }

  int RequestPool::GetCountByStateExceptions(std::initializer_list<Request::State> state_list)
  {
    return std::count_if(pool.begin(), pool.end(), [&] (Request::Ptr request) -> bool
    {
      return std::none_of(state_list.begin(), state_list.end(), [&] (Request::State state_exception) -> bool
      {
        return request->GetState() == state_exception;
      });
    });
  }

  std::pair< std::set<Request::Ptr>::iterator, bool > RequestPool::AddRequest(Request::Ptr request)
  {
    return pool.emplace(request);
  }

  std::set<Request::Ptr> &RequestPool::Get() noexcept
  {
    return pool;
  }
}