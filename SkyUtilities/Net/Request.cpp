#include "Net/Request.h"
#include "Plugin.h"

#include <exception>

namespace SKU::Net {

	static unsigned GLOBAL_REQUEST_ID_COUNTER = 0;

	Request::Request() noexcept
		: state(sWaitingForSetup)
	{
		id = ++GLOBAL_REQUEST_ID_COUNTER;
	}

	Request::~Request()
	{
		Stop();
	}

	void Request::Stop()
	{
		if (proto_ctx == nullptr)
		{
			return;
		}

		Plugin::Log(LOGL_VERBOSE, "Request (id: %d): Stopping request.", id);

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

	void Request::SetState(State state) noexcept
	{
		this->state = state;
	}

	void Request::SetTimeout(unsigned ms) noexcept
	{
		timeout = ms;
	}

}
