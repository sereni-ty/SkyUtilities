#pragma once

#include <string>
#include <functional>
#include <memory>

#include <boost/property_tree/ptree.hpp>

namespace SKU {
  class Configuration
  {
    public:
    template<class T>
    struct Setting
    {
      Setting(const std::string &key, const T &default_value, std::function<T(const T&)> value_limits_check)
        : key(key), default_value(default_value), value_limits_check(value_limits_check)
      {}

      T operator ()(const T &value)
      {
        return value_limits_check(value);
      }

      std::string key;
      T default_value;
      std::function<T(const T&)> value_limits_check;
    };

    public:
    using Ptr = std::unique_ptr<Configuration>;

    public:
    Configuration(const std::string &path);
    ~Configuration();

    private:
    void Load();

    void Save();

    public:
    template<typename T>
    bool Get(const Setting<T> &setting, T &value);

    template<typename T>
    void Set(const Setting<T> &setting, const T &value);

    template<typename T>
    void SetInitial(const Setting<T> &setting);

    private:
    boost::property_tree::ptree json_values;
    std::string path;
  };

# include "Configuration.inl"
}
