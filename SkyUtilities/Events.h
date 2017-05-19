#pragma once

#include <skse/PluginAPI.h>

#include <skse/PapyrusArgs.h>

namespace SKU {
	
	class IEventHandler
	{
		friend class Plugin;

		protected:
			//
			// Papyurs
			virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry*) {}

			//
			// Messaging
			virtual void OnSKSEMessage(SKSEMessagingInterface::Message*) {}

			//
			// Serialization
			virtual void OnSKSESaveGame(SKSESerializationInterface*) {}
			virtual void OnSKSELoadGame(SKSESerializationInterface*) {}
	};
}
