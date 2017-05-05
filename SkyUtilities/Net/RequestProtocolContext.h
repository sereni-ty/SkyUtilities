#pragma once

#include <memory>
#include <string>

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

		public:
			virtual std::string GetResponse() { return response; }

		protected:
			std::string response;

		private:
			std::shared_ptr<Request> owner;

		friend class Request;
	};

}}
