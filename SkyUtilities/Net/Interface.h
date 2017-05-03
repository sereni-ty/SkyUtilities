#pragma once

#include "Singleton.h"
#include "Events.h"

#include "Net/HTTP/RequestProtocolContext.h"

struct StaticFunctionTag;

namespace SKU { namespace Net {

	class Interface : public Singleton<Interface>, public IEventHandler
	{
		IS_SINGLETON_CLASS(Interface)

		public:
			void Stop(); // Stop every net activity

		public:
			static long HTTPGETRequest(StaticFunctionTag*, BSFixedString url, long timeout);
			static long HTTPPOSTRequest(StaticFunctionTag*, BSFixedString url, BSFixedString body, long timeout);

			static long HTTPRequest(HTTP::RequestProtocolContext::Method method, std::string url, std::string body, long timeout);

		public:
			void OnSKSERegisterPapyrusFunctions(VMClassRegistry *registry);

		private:
			bool stopped;
	};

}}
