#pragma once

#include "Net/Request.h"

#include <set>
#include <initializer_list>
#include <algorithm>
#include <vector>

namespace SKU::Net {
  class RequestPool
  {
    public:
    Request::Ptr GetRequestByID(int id);

    public:
    std::vector<Request::Ptr> GetRequestsByState(Request::State state);

    public:
    int GetCountByState(Request::State state);
    int GetCountByStateExceptions(std::initializer_list<Request::State> state_list);

    public:
    std::set<Request::Ptr> &Get() noexcept;

    private:
    std::set<Request::Ptr> pool;
  };
}
