#pragma once

#include "Singleton.h"
#include "Events.h"

#include "Net/HTTP/RequestProtocolContext.h"

struct StaticFunctionTag;
class TESForm;

namespace SKU::Net {

	class Interface : public Singleton<Interface>, public IEventHandler
	{
		IS_SINGLETON_CLASS(Interface)

		public:
			void Stop(); // Stop every net activity

		public:
			static long HTTPGETRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, long timeout);
			static long HTTPPOSTRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, BSFixedString body, long timeout);

			static long HTTPRequest(TESForm *form, HTTP::RequestProtocolContext::Method method, std::string url, std::string body, long timeout);

			static BSFixedString URLEncode(StaticFunctionTag*, BSFixedString raw);

		public:
			void OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept;
			void OnSKSESaveGame(SKSESerializationInterface *serialization_interface);
			void OnSKSELoadGame(SKSESerializationInterface *serialization_interface);

		public:
			enum PapyrusEvent
			{
				evHTTPRequestFinished
			};

			static std::string GetEventString(PapyrusEvent event) noexcept;

		private:
			bool stopped;
	};

}
