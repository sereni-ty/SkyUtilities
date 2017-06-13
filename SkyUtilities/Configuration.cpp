#include "Configuration.h"
#include "Plugin.h"

#include <fstream>
#include <boost/property_tree/json_parser.hpp>

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

  void Configuration::Load()
  {
    std::string line;
    std::ifstream confs(path);

    if (confs.is_open() == false)
    {
      return;
    }

    Plugin::Log(LOGL_VERBOSE, "Configuration: Loading (%s)..", path.c_str());

    try
    {
      boost::property_tree::read_json(confs, json_values);
    }
    catch (boost::property_tree::json_parser_error &e)
    {
      Plugin::Log(LOGL_VERBOSE, "Configuration: Failed parsing (%s)", e.what());
    }

    Plugin::Log(LOGL_VERBOSE, "Configuration: Ready (%d).", json_values.size());
  }

  void Configuration::Save()
  {
    Plugin::Log(LOGL_VERBOSE, "Configuration: Attempting to save (%s).", path.c_str());

    if (json_values.empty() == true)
    {
      return;
    }

    if (path.empty() == true)
    {
      Plugin::Log(LOGL_VERBOSE, "Configuration: Invalid path. Unable to save.");
      return;
    }

    std::ofstream confs(path);

    if (confs.is_open() == false)
    {
      Plugin::Log(LOGL_VERBOSE, "Configuration: Couldn't open configuration file to save data.");
      return;
    }

    try
    {
      boost::property_tree::write_json(confs, json_values);
    }
    catch (boost::property_tree::json_parser_error &e)
    {
      Plugin::Log(LOGL_VERBOSE, "Configuration: Failed writing configuration  (%s).", e.what());
    }
  }
}