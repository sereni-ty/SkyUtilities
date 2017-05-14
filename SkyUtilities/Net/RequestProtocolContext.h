#pragma once

#include <memory>
#include <string>

namespace SKU::Net {
	
	class Request;

	class IRequestProtocolContext
	{
		public:
			using Ptr = std::shared_ptr<IRequestProtocolContext>;

		public:	
			virtual ~IRequestProtocolContext() { Cleanup(); if(owner) owner.reset(); }

		protected:
			void SetOwner(std::shared_ptr<Request> owner) noexcept { this->owner = owner; }
			std::shared_ptr<Request> GetOwner() const noexcept { return this->owner; }

		public:
			virtual void Initialize() {};
			virtual void Cleanup() {};

		public:
			virtual std::string GetResponse() noexcept { return response; }

		protected:
			std::string response;

		private:
			std::shared_ptr<Request> owner;

		friend class Request;
	};

}
