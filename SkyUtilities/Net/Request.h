#pragma once

#include "Lockable.h"
#include "Net/RequestProtocolContext.h"

#include <string>
#include <memory>
#include <mutex>

namespace SKU::Net {	// TODO: Serialization. Save / Load undone requests from savegame __BUT__ check first if that is even necessary if only the event gets the response and there is basically no interaction between papyrus and this interface besides starting a request and setting up an event callback function

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
			Request() noexcept;
			~Request();

		public:
			template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
			static Ptr Create();

		public:
			void Stop();

		public:
			int GetID() noexcept;
			State GetState() noexcept;
			unsigned GetTimeout() noexcept; // Timeout in ms

			template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
			typename ProtocolContextType::Ptr GetProtocolContext();

		public:
			void SetState(State state) noexcept;
			void SetTimeout(unsigned ms) noexcept;

		private:
			int id;
			State state;
			IRequestProtocolContext::Ptr proto_ctx;
			unsigned timeout;

			std::mutex mtx;
	};

#	include "Request.inl"

}
