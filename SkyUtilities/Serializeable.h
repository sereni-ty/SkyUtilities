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
    virtual bool IsRequestedSerialization(SerializationEntity &serialized)
    {
      return false;
    };

    protected:
    template<class T = std::is_integral<T>>
    inline void SerializeIntegral(SerializationEntity &serialized, T value)
    {
      std::get<ISerializeable::idStream>(serialized).write(reinterpret_cast<char *>(&value), sizeof(value));
    };

    inline void SerializeString(SerializationEntity &serialized, std::string value)
    {
      SerializeIntegral(serialized, value.length());
      std::get<ISerializeable::idStream>(serialized).write(value.c_str(), value.length());
    }

    template<class T = std::is_integral<T>>
    inline void DeserializeIntegral(SerializationEntity &serialized, T &value)
    {
      std::get<ISerializeable::idStream>(serialized).read(reinterpret_cast<char *>(&value), sizeof(value));
    };

    inline void DeserializeString(SerializationEntity &serialized, std::string &value)
    {
      size_t length = 0;
      char c;

      value.clear();

      DeserializeIntegral(serialized, length);

      while (length-- > 0)
      {
        std::get<ISerializeable::idStream>(serialized).get(c);
        value.push_back(c);
      }
    }
  };
}