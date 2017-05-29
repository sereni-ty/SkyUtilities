#pragma once

#include <memory>
#include <type_traits>

#define IS_SINGLETON_CLASS(ClassName)\
	friend class Singleton<ClassName>;\
	protected:\
		ClassName();\
		~ClassName();

namespace SKU {
  template< class T = std::is_class<T> >
  class Singleton
  {
    friend T;

    protected:
    Singleton() noexcept
    {}
    ~Singleton()
    {}

    public:
    static T *GetInstance() noexcept
    {
      static T instance; return &instance;
    }
  };
}
