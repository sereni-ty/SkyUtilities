#pragma once

#include <sstream>
#include <tuple>
#include <stack>
#include <vector>

namespace SKU {
  class ISerializeable
  {
    friend class Plugin;

    public:
    using SerializationEntity = std::tuple<uint32_t /* type */, uint32_t /* version */, std::stringstream /* data */>;

    public:
    const enum
    {
      idType = 0,
      idVersion,
      idStream,
    };

    public:
    virtual void Serialize(std::stack<SerializationEntity> &serialized_entities) = 0;
    virtual void Deserialize(SerializationEntity &serialized) = 0;

    public:
    virtual bool IsRequestedSerialization(const SerializationEntity &serialized)
    {
      return false;
    }

    protected:
    template<class T>
    inline void SerializeIntegral(SerializationEntity &serialized, T value) noexcept
    {
      if (std::is_integral<T>::value == true)
      {
        try
        {
          std::get<idStream>(serialized).write(reinterpret_cast<char *>(&value), sizeof(value));
        }
        catch (std::exception)
        {
        }
      }
    }

    inline void ISerializeable::SerializeString(SerializationEntity &serialized, std::string value) noexcept
    {
      SerializeIntegral(serialized, value.length());

      try
      {
        std::get<idStream>(serialized).write(value.c_str(), value.length());
      }
      catch (std::exception)
      {
      }
    }

    template<class T>
    inline void DeserializeIntegral(SerializationEntity &serialized, T &value) noexcept
    {
      if (std::is_integral<T>::value == true)
      {
        try
        {
          std::get<idStream>(serialized).read(reinterpret_cast<char *>(&value), sizeof(value));
        }
        catch (std::exception)
        {
        }
      }
    }

    inline void DeserializeString(SerializationEntity &serialized, std::string &value) noexcept
    {
      size_t length = 0;
      char c;

      try
      {
        value.clear();

        DeserializeIntegral(serialized, length);

        while (length-- > 0)
        {
          std::get<idStream>(serialized).get(c);
          value.push_back(c);
        }
      }
      catch (std::exception)
      {
      }
    }
  };
}