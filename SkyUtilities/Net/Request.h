#pragma once

#include "Lockable.h"
#include "Net/RequestProtocolContext.h"

#include <string>
#include <memory>
#include <mutex>

namespace SKU::Net {

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
			Request(int pre_set_id = -1) noexcept;
			~Request();

		public:
			template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
			static Ptr Create(int pre_set_id = -1);

		public:
			void Stop();

		public:
			int GetID() noexcept;
			State GetState() noexcept;
			unsigned GetTimeout() noexcept; // Timeout in ms

			template< class ProtocolContextType = std::is_base_of<IRequestProtocolContext, ProtocolContextType> >
			typename ProtocolContextType::Ptr GetProtocolContext();

		public:
			void SetID(int id) noexcept;
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
