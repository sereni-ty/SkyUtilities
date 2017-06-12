#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <memory>

// TODO: JSON
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
    static bool ParseLine(const std::string &line, std::string &key, std::stringstream &value);

    void Save();

    public:
    template<typename T>
    bool Get(const Setting<T> &setting, T &value);

    template<typename T>
    void Set(const Setting<T> &setting, const T &value);

    template<typename T>
    void SetInitial(const Setting<T> &setting);

    private:
    std::unordered_map<std::string /* key */, std::stringstream /* value */> entries;
    std::string path;
  };

# include "Configuration.inl"
}
