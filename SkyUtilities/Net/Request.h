#pragma once

#include "Lockable.h"
#include "Net/RequestProtocolContext.h"

#include <string>
#include <memory>
#include <mutex>

namespace SKU { namespace Net {	// TODO: Serialization. Save / Load undone requests from savegame __BUT__ check first if that is even necessary if only the event gets the response and there is basically no interaction between papyrus and this interface besides starting a request and setting up an event callback function

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
	};

#	include "Request.inl"

}}
