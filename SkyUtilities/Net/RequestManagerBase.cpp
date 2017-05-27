#include "Net/RequestManagerBase.h"

#include "Plugin.h"

namespace SKU::Net {
	
	bool RequestManagerBase::AddRequest(Request::Ptr request) noexcept
	{
		try
		{
			pool.Get().insert(request);
		} 
		catch (std::exception e)
		{
			return false;
		}

		OnRequestAdded(request);

		return true;
	}

	void RequestManagerBase::RemoveRequest(Request::Ptr request)
	{
		OnRequestRemoval(request);

		try
		{
			pool.Get().erase(request);
		}
		catch (std::exception){}
	}

	Request::Ptr RequestManagerBase::GetRequestByID(unsigned request_id) noexcept
	{
		try
		{
			return pool.GetRequestByID(request_id);
		}
		catch(std::exception){}

		return nullptr;
	}
}
