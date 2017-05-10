#pragma once

#include "Singleton.h"
#include "Events.h"

#include "Net/HTTP/RequestProtocolContext.h"

struct StaticFunctionTag;
class TESForm;

namespace SKU { namespace Net {

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
			static BSFixedString GetBufferedData(StaticFunctionTag*, long request_id);

		public:
			void OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry);

		public:
			enum PapyrusEvent
			{
				evRequestFinished
			};

			static std::string GetEventString(PapyrusEvent event);

		private:
			bool stopped;
	};

}}
