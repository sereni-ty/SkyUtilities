#pragma once

#include <skse/PluginAPI.h>

#include <skse/PapyrusArgs.h>

namespace SKU {
	
	class IEventHandler
	{
		friend class Plugin;

		protected:
			virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry*) {}
			virtual void OnSKSEMessage(SKSEMessagingInterface::Message*) {}
	};
}
