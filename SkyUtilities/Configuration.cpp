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
    std::ifstream confs(path);

    if (confs.is_open() == false)
    {
      return;
    }

    try
    {
      boost::property_tree::read_json(confs, json_values);
    }
    catch (boost::property_tree::json_parser_error)
    {
    }
  }

  void Configuration::Save()
  {
    if (path.empty() == true || json_values.empty() == true)
    {
      return;
    }

    std::ofstream confs(path);

    if (confs.is_open() == false)
    {
      return;
    }

    try
    {
      boost::property_tree::write_json(confs, json_values);
    }
    catch (boost::property_tree::json_parser_error)
    {
    }
  }
}