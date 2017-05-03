#pragma once

#include "Singleton.h"
#include "RequestPool.h"

namespace SKU { namespace Net {

	class RequestManagerBase
	{
		public:
			bool AddRequest(Request::Ptr request, bool proccess_immediately = false);
			virtual void RemoveRequest(Request::Ptr request);

		public:
			virtual void Stop() = 0;
			virtual void Start() = 0;

		protected:
			RequestPool pool;
	};

}}
