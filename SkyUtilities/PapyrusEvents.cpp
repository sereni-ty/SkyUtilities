#include "PapyrusEvents.h"
#include "Plugin.h"

#include <utility>

// TODO: Not finished yet. Unsecure. Most probably will be restructured completeley. 

namespace SKU {
	
	std::map< std::string, std::queue<uint64_t> > PapyrusEvent::registered_listener;

	PapyrusEvent::PapyrusEvent(const std::string &event_name)
		: event_name(event_name)
	{}

	PapyrusEvent::~PapyrusEvent()
	{
		VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();
		IObjectHandlePolicy *policy = registry->GetHandlePolicy();

		if (registered_listener.empty() == false)
		{
			for (auto &event_queue : registered_listener)
			{
				while (event_queue.second.empty() == false)
				{
					policy->Release(event_queue.second.front());
					event_queue.second.pop();
				}
			}
		}
	}

	PapyrusEvent& PapyrusEvent::SetArgument(bool value)
	{
		arguments.emplace_back(std::make_pair(VMValue::kType_Bool, value));
		return *this;
	}

	PapyrusEvent& PapyrusEvent::SetArgument(long value)
	{
		arguments.emplace_back(std::make_pair(VMValue::kType_Int, value));
		return *this;
	}

	PapyrusEvent& PapyrusEvent::SetArgument(float value)
	{
		arguments.emplace_back(std::make_pair(VMValue::kType_Float, value));
		return *this;
	}

	PapyrusEvent& PapyrusEvent::SetArgument(std::string &value)
	{
		arguments.emplace_back(std::make_pair(VMValue::kType_String, (long)value.c_str()));
		return *this;
	}

	PapyrusEvent& PapyrusEvent::SetArgument(VMClassInfo *value)
	{
		//arguments.emplace_back(std::make_pair(VMValue::kType_Identifier, value));
		return *this;
	}

	void PapyrusEvent::RegisterListener(const std::string& event_name, TESForm *form)
	{
		VMClassRegistry		* registry = (*g_skyrimVM)->GetClassRegistry();
		IObjectHandlePolicy	* policy = registry->GetHandlePolicy();

		if (form == nullptr)
		{
			Plugin::Log(LOGL_INFO, "PapyrusEvent: Could register event because form handle was invalid.");
			return;
		}

		uint64_t handle = policy->Create(form->formType, form);

		if (handle == policy->GetInvalidHandle())
		{
			Plugin::Log(LOGL_INFO, "PapyrusEvent: Unable to register listener to event '%s'.",
				event_name.c_str());

			return;
		}
		
		if (registered_listener.find(event_name) == registered_listener.end())
		{
			registered_listener.emplace(event_name, std::queue<uint64_t>());

			Plugin::Log(LOGL_INFO, "PapyrusEvent: Event '%s' registered.", event_name.c_str());
		}

		registered_listener.at(event_name).push(handle);
	}

	void PapyrusEvent::Send()
	{
		VMClassRegistry *registry = (*g_skyrimVM)->GetClassRegistry();

		auto event_registration_queue = registered_listener.find(event_name);

		if (event_registration_queue == registered_listener.end())
		{
			Plugin::Log(LOGL_VERBOSE, "PapyursEvent: No registered listeners found for event '%s'.", event_name.c_str());
			return;
		}

		Plugin::Log(LOGL_VERBOSE, "PapyrusEvent: Sending event '%s'.", event_name.c_str());

		while((*event_registration_queue).second.empty() == false)
		{
			registry->QueueEvent((*event_registration_queue).second.front(), &BSFixedString(event_name.c_str()), this);
			(*event_registration_queue).second.pop();
		}
	}

	bool PapyrusEvent::Copy(Output * dst)
	{
		dst->Resize(arguments.size());
		
		for (unsigned i = 0; i < arguments.size(); i++)
		{
			switch (arguments.at(i).first)
			{
				case VMValue::kType_Bool: dst->Get(i)->SetBool(arguments.at(i).second); break;
				case VMValue::kType_Int: dst->Get(i)->SetInt(arguments.at(i).second); break;
				case VMValue::kType_Float: dst->Get(i)->SetFloat(arguments.at(i).second); break;
				case VMValue::kType_String: dst->Get(i)->SetString((const char *)arguments.at(i).second); break;
				//case VMValue::kType_Identifier: dst->Get(i)->SetIdentifier(((VMIdentifier**)&(arguments.at(i).second))); break;
			}
		}

		return true;
	}
}
