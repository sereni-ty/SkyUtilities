#pragma once

#include <skse/PluginAPI.h>
#include <skse/PapyrusArgs.h>

#define FAIL_BREAK_WRITE(iface, data, len)	if((iface)->WriteRecordData((data), (len)) == false) break;
#define FAIL_BREAK_READ(iface, data, len)	if((iface)->ReadRecordData((data), (len)) == 0) break;

namespace SKU {
	
	class IEventHandler
	{
		friend class Plugin;

		protected:
			//
			// Papyrus
			virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry*) {}

			//
			// Messaging
			virtual void OnSKSEMessage(SKSEMessagingInterface::Message*) {}

			//
			// Serialization
			virtual void OnSKSESaveGame(SKSESerializationInterface*) {}
			virtual void OnSKSELoadGame(SKSESerializationInterface*, SInt32 type, SInt32 version, SInt32 length) {}
			virtual void OnSKSERevertGame(SKSESerializationInterface*) {} // TODO: Check if records are overwriteable 
	};
}
