#pragma once

#include "Singleton.h"
#include "Events.h"

#include "Net/HTTP/RequestProtocolContext.h"

struct StaticFunctionTag;
class TESForm;

#define REQUESTS_LIMIT_PER_TIMEFRAME 5
#define REQUESTS_LIMIT_TIMEFRAME 1000 // ms 
#define REQUESTS_LIMIT_EXCEEDINGS_PERMITTED 3

namespace SKU::Net {

	class Interface : public Singleton<Interface>, public IEventHandler
	{
		IS_SINGLETON_CLASS(Interface)

		public:
			void Stop(); // Stop every net activity

		public:
			static long HTTPGETRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, long timeout);
			static long HTTPPOSTRequest(StaticFunctionTag*, TESForm *form, BSFixedString url, BSFixedString body, long timeout);

			static long GetNexusModInfo(StaticFunctionTag*, TESForm *form, BSFixedString mod_id);
			static long GetLLabModInfo(StaticFunctionTag*, TESForm *form, BSFixedString mod_id);

			static long HTTPRequest(uint32_t request_handler_type_id, TESForm *form, HTTP::RequestProtocolContext::Method method, std::string url, std::string body, long timeout);

			static BSFixedString URLEncode(StaticFunctionTag*, BSFixedString raw);
			static BSFixedString URLDecode(StaticFunctionTag*, BSFixedString encoded);

		public:
			virtual void OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry) noexcept final;
			
		public:
			enum PapyrusEvent
			{
				evHTTPRequestFinished,
				evModInfoRetrieval
			};

			static std::string GetEventString(PapyrusEvent event) noexcept;

		private:
			bool stopped;
	};

}
