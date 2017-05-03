#pragma once

#include <skse/PapyrusArgs.h>
#include <skse/PluginAPI.h>

namespace SKU {
	
	//class IEventDistributor {};

	class IEventHandler
	{
		//friend class IEventDistributor;
		friend class Plugin;

		protected:
			virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry*) {}
			virtual void OnSKSEMessage(SKSEMessagingInterface::Message*) {}
	};

}
