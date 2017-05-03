#pragma once

#include <mutex>

namespace SKU {
	
	class Lockable // For the time being sufficient..
	{
		public:
			void Lock()
			{ 
				mtx.lock(); 
			}

			void Unlock() 
			{
				mtx.unlock(); 
			}

		protected:
			std::mutex mtx;
	};

}
