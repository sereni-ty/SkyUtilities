#include "Net/Request.h"
#include "Plugin.h"

#include <exception>

namespace SKU { namespace Net {

	static unsigned GLOBAL_REQUEST_ID_COUNTER = 0;

	Request::Request()
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

	int Request::Request::GetID()
	{
		return id;
	}

	Request::State Request::GetState()
	{
		return state;
	}

	unsigned Request::GetTimeout()
	{
		return timeout;
	}	

	void Request::SetState(State state)
	{
		this->state = state;
	}

	void Request::SetTimeout(unsigned ms)
	{
		timeout = ms;
	}

}}
