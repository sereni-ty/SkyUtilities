#include "PapyrusEvent.h"

#include "PapyrusEventHandler.h"
#include "Plugin.h"

#include <utility>
#include <typeinfo>

namespace SKU {

	PapyrusEvent::PapyrusEvent(Args &&args)
	{
		arguments = std::move(args);
	}

	PapyrusEvent::~PapyrusEvent()
	{

	}

	bool PapyrusEvent::Copy(Output * dst)
	{
		dst->Resize(arguments.size());

		Plugin::Log(LOGL_VERBOSE, "PapyrusEvent: Arguments (count: %d) for papyrus event are being set up.", 
			arguments.size());

		for (unsigned i = 0; i < arguments.size(); i++)
		{
			// TODO: Missing types. Common ones are handled.
			size_t arg_type = arguments.at(i).type().hash_code();
			
			if (arg_type == typeid(bool).hash_code()) 
			{ 
				auto value = std::any_cast<bool>(arguments.at(i));
				Plugin::Log(LOGL_DETAILED, "PapyrusEvent: Argument #%d is '%s' (bool).", i+1, value == true ? "true" : "false");
				dst->Get(i)->SetBool(value); 
			}
			else if (arg_type == typeid(int).hash_code()) 
			{
				auto value = std::any_cast<int>(arguments.at(i));
				Plugin::Log(LOGL_DETAILED, "PapyrusEvent: Argument #%d is '%d' (int).", i + 1, value);
				dst->Get(i)->SetInt(value);
			}
			else if (arg_type == typeid(float).hash_code())
			{
				auto value = std::any_cast<float>(arguments.at(i));
				Plugin::Log(LOGL_DETAILED, "PapyrusEvent: Argument #%d is '%f' (float).", i + 1, value);
				dst->Get(i)->SetFloat(value);
			}
			else if (arg_type == typeid(std::string).hash_code())
			{
				auto value = std::any_cast<std::string>(arguments.at(i));
				Plugin::Log(LOGL_DETAILED, "PapyrusEvent: Argument #%d is a string (std::string).", i + 1);
				dst->Get(i)->SetString(value.c_str());
			}
			else if (arg_type == typeid(char*).hash_code())
			{
				auto value = std::any_cast<char *>(arguments.at(i));
				Plugin::Log(LOGL_DETAILED, "PapyrusEvent: Argument #%d is a string (char*).", i + 1);
				dst->Get(i)->SetString(value);
			}
		}

		return true;
	}
}
