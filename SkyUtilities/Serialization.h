#pragma once

#include <skse/PluginAPI.h>

#define FAIL_BREAK_WRITE(iface, data, len) if((iface)->WriteRecordData((data), (len)) == false) break;
#define FAIL_BREAK_READ(iface, data, len) if((iface)->ReadRecordData((data), (len)) != (len)) break;

namespace SKU
{
	class ISerializeable
	{
		public:
			virtual void Save(SKSESerializationInterface *serilization_interface) = 0;
			virtual void Load(SKSESerializationInterface *serilization_interface) = 0;
	};

}
