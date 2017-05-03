#pragma once

#include <memory>

namespace SKU { namespace Net {
	
	class Request;

	class IRequestProtocolContext
	{
		public:
			using Ptr = std::shared_ptr<IRequestProtocolContext>;

		public:	
			virtual ~IRequestProtocolContext() { Cleanup(); if(owner) owner.reset(); }

		protected:
			void SetOwner(std::shared_ptr<Request> owner) { this->owner = owner; }
			std::shared_ptr<Request> GetOwner() const { return this->owner; }

		public:
			virtual void Initialize() {};
			virtual void Cleanup() {};

		private:
			std::shared_ptr<Request> owner;

		friend class Request;
	};

}}
