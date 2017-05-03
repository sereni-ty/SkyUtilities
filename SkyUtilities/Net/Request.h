#pragma once

#include "Lockable.h"
#include "Net/RequestProtocolContext.h"

#include <string>
#include <memory>
#include <mutex>

namespace SKU { namespace Net {	

	class Request : public Lockable
	{
		public:
			using Ptr = std::shared_ptr<Request>;

		public:
			enum State
			{
				sFailed = 0,
				sOK = 1,
				sWaitingForSetup,
				sReady,
				sPending,
			};

		public:
			Request();
			~Request();

		public:
			template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
			static Ptr Create();

		public:
			void Stop();

		public:
			int GetID();
			State GetState();
			unsigned GetTimeout(); // Timeout in ms

			template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
			typename ProtocolContextType::Ptr GetProtocolContext();

		public:
			void SetState(State state);
			void SetTimeout(unsigned ms);

		private:
			int id;
			State state;
			IRequestProtocolContext::Ptr proto_ctx;
			unsigned timeout;

			std::mutex mtx;

		friend class Interface;
		friend class RequestPool;
		friend class RequestManager;
	};

#	include "Request.inl"

}}
