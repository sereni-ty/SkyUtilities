#pragma once

#include <string>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <memory>

namespace SKU {
  class Configuration
  {
    public:
    using Ptr = std::unique_ptr<Configuration>;

    public:
    Configuration(const std::string &path);
    ~Configuration();

    private:
    void Load();
    bool ParseLine(const std::string &line, std::string &key, std::stringstream &value);

    void Save();

    public:
    template<typename T>
    bool Get(const std::string &key, T &value); // TODO: default value

    template<typename T>
    bool Get(const std::string &key, T &value, std::function<T(const T&)> value_limits_check); // TODO: default value

    template<typename T>
    void Set(const std::string &key, const T &value);

    template<typename T>
    void SetInitial(const std::string &key, const T &value);

    private:
    std::unordered_map<std::string /* key */, std::stringstream /* value */> entries;
    std::string path;
  };

# include "Configuration.inl"
}
