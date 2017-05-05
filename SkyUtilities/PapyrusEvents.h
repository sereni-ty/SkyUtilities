#pragma once

#include <skse/PapyrusVM.h>
#include <skse/PapyrusObjects.h>
#include <skse/PapyrusEvents.h>

#include <memory>
#include <vector>
#include <map>
#include <queue>
#include <string>

namespace SKU
{
	class PapyrusEvent : public IFunctionArguments
	{
		public:
			PapyrusEvent(const std::string &event_name);
			~PapyrusEvent();

		public:
			PapyrusEvent& SetArgument(bool value);
			PapyrusEvent& SetArgument(long value);
			PapyrusEvent& SetArgument(float value);
			PapyrusEvent& SetArgument(std::string &value);
			PapyrusEvent& SetArgument(VMClassInfo *value);

		public:
			static void RegisterListener(const std::string &event_name, TESForm *form);
			void Send();

		// SKSE internal 
		public:
			bool Copy(Output * dst);

		private:
			std::string event_name;
			std::vector< std::pair< char, long > > arguments;

			static std::map< std::string, std::queue<uint64_t> > registered_listener; // queue holds papyrus script instance form handles
	};
}
