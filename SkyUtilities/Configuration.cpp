#include "Configuration.h"
#include "Plugin.h"

#include <fstream>
#include <cctype>

namespace SKU {
  Configuration::Configuration(const std::string &path)
    : path(path)
  {
    Load();
  }

  Configuration::~Configuration()
  {
    Save();
  }

  void Configuration::Load() // TODO: doesn't work properly..
  {
    std::ifstream confs(path);
    std::string line;

    if (confs.is_open() == false)
    {
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "Configuration: Loading..");

    while (std::getline(confs, line).eof() == false)
    {
      std::string key;
      std::stringstream value;

      if (ParseLine(line, key, value) == true)
      {
        Plugin::Log(LOGL_VERBOSE, "Configuration: Loaded '%s' = '%s'",
          key.c_str(), value.str().c_str());

        entries.try_emplace(key, std::move(value));
      }
    }

    Plugin::Log(LOGL_VERBOSE, "Configuration: Ready.");
  }

  bool Configuration::ParseLine(const std::string &line, std::string &key, std::stringstream &value)
  {
    std::stringstream splitter(line);
    std::string tmp;

    splitter >> key;
    splitter >> tmp;

    if (tmp != "=")
    {
      return false;
    }

    value = std::stringstream(splitter.str().substr(splitter.tellg()));

    while (value.str().empty() == false && std::isspace(value.str().front()))
    {
      value.str().erase(0);
    }

    return true;
  }

  void Configuration::Save()
  {
    if (entries.empty() == true || path.empty() == true)
    {
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "Configuration: Saving..");

    std::ofstream confs(path);

    if (confs.is_open() == false)
    {
      Plugin::Log(LOGL_INFO, "Configuration: Couldn't save configuration file.");
      return;
    }

    size_t longest_key_length = 0;

    for (auto &entry : entries)
    {
      longest_key_length = std::max<size_t>(entry.first.length(), longest_key_length);
    }

    for (auto &entry : entries)
    {
      confs.width(longest_key_length + 1); // to maximize readability
      confs << std::left << entry.first;

      confs.width(1);
      confs << "= ";
      confs << entry.second.str();
      confs << std::endl;
    }

    Plugin::Log(LOGL_VERBOSE, "Configuration: Done.");
  }
}