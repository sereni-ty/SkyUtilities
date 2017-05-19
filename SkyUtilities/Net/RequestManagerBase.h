#pragma once

#include "Singleton.h"

#include "Net/RequestPool.h"

namespace SKU::Net {

	class RequestManagerBase
	{
		public:
			bool AddRequest(Request::Ptr request, bool proccess_immediately = false) noexcept;
			virtual void RemoveRequest(Request::Ptr request);
			Request::Ptr GetRequestByID(unsigned request_id) noexcept;

		public:
			virtual void Stop() = 0;
			virtual void Start() = 0;

		protected:
			RequestPool pool;
	};

}
