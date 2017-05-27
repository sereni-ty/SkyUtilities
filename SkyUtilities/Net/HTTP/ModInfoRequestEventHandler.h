#pragma once
#include "Net/Request.h"
#include "Net/RequestEventHandler.h"

namespace SKU::Net::HTTP {

	class ModInfoRequestEventHandler : public RequestEventHandler
	{
		public:
			using Ptr = std::shared_ptr<ModInfoRequestEventHandler>;

		public:
			virtual void OnRequestFinished(Request::Ptr request) final;

		public:
			virtual uint32_t TypeID() final { return 2; } // TODO: I know, I know..
	};

}
