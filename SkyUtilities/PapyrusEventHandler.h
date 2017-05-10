#pragma once

#include "Singleton.h"
#include "PapyrusEvent.h"

#include <skse/PapyrusVM.h>
#include <skse/PapyrusObjects.h>
#include <skse/PapyrusEvents.h>

#include <unordered_map>
#include <unordered_set>

#include <vector>
#include <any>
#include <string>

namespace SKU {
	
	using PapyrusEventRecipient = uint64_t;

	class PapyrusEventHandler : public Singleton<PapyrusEventHandler>
	{
		IS_SINGLETON_CLASS(PapyrusEventHandler)

		friend class PapyrusEvent;

		public:
			bool Send(const std::string &event_name, PapyrusEvent::Args &&args);
						
		public:
			void Register(const std::string &event_name);
			void Unregister(const std::string &event_name);
			void UnregisterAll();

		protected:
			void Cleanup(const PapyrusEvent *event);

		public:
			bool AddRecipient(const std::string &event_name, TESForm *recipient);
			
			//void RemoveRecipient(const std::string &event_name, TESForm *recipient);
			//void RemoveRecipientEntirely(TESForm *recipient);
			void RemoveRecipients();

		private:
			using RecipientMap = std::unordered_map</* event name: */std::string, std::unordered_set< PapyrusEventRecipient > >;
			using EventMap = std::unordered_map </* event name: */std::string, std::unordered_set< std::unique_ptr<PapyrusEvent> > >;
			using EventKeyMap = std::unordered_map < std::string, BSFixedString >;

			RecipientMap recipient_map;
			EventMap event_map;
			EventKeyMap event_key_map;
	};

}
