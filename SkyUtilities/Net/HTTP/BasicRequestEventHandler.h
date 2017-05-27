#pragma once

#include "Net/Request.h"
#include "Net/RequestEventHandler.h"

namespace SKU::Net::HTTP {

	class BasicRequestEventHandler : public RequestEventHandler
	{
		public:
			using Ptr = std::shared_ptr<BasicRequestEventHandler>;

		public:
			virtual void OnRequestFinished(Request::Ptr request) final;

		public:
			virtual uint32_t TypeID() final { return 1; } // TODO: I know, I know..
	};

}
