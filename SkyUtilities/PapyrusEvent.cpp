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
			// TODO: check what about std::any_cast does not work like it should. Call works fine.
			size_t arg_type = arguments.at(i).type().hash_code();

			if (arg_type == typeid(bool).hash_code()) dst->Get(i)->SetBool(std::any_cast<bool>(arguments.at(i)));
			else if (arg_type == typeid(int).hash_code()) dst->Get(i)->SetInt(std::any_cast<long>(arguments.at(i)));
			else if (arg_type == typeid(float).hash_code()) dst->Get(i)->SetFloat(std::any_cast<float>(arguments.at(i)));
			else if (arg_type == typeid(std::string).hash_code()) dst->Get(i)->SetString(std::any_cast<std::string>(arguments.at(i)).c_str());
			else if (arg_type == typeid(char*).hash_code()) dst->Get(i)->SetString(std::any_cast<char*>(arguments.at(i)));
		}

		return true;
	}
}
