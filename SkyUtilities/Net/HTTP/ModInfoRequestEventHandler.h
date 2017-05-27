#pragma once
#include "Net/Request.h"
#include "Net/RequestEventHandler.h"

namespace SKU::Net::HTTP {

	class NexusModInfoRequestEventHandler : public RequestEventHandler
	{
		public:
			using Ptr = std::shared_ptr<NexusModInfoRequestEventHandler>;

		public:
			virtual void OnRequestFinished(Request::Ptr request) final;

		public:
			static uint32_t TypeID() { return 2; } // TODO: I know, I know..
	};

	class LLabModInfoRequestEventHandler : public RequestEventHandler
	{
		public:
			using Ptr = std::shared_ptr<LLabModInfoRequestEventHandler>;

		public:
			virtual void OnRequestFinished(Request::Ptr request) final;

		public:
			static uint32_t TypeID() { return 3; } // TODO: I know, I know..
	};

}
