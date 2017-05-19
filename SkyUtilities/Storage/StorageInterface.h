#pragma once

#include "Singleton.h"
#include "Events.h"

namespace SKU::Storage {

	class Interface : public Singleton<Interface>, public IEventHandler
	{
		IS_SINGLETON_CLASS(Interface)

		public:
			void OnSKSEMessage(SKSEMessagingInterface::Message*);					
	};
}
