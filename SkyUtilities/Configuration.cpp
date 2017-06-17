#include "Configuration.h"
#include "Plugin.h"

#include <fstream>
#include <boost/property_tree/json_parser.hpp>

#include <windows.h>

namespace SKU {
  Configuration::Configuration(const std::string &path)
    : path(path)
    , last_changed(0)
  {
    Load();
  }

  Configuration::~Configuration()
  {
    Save();
  }

  void Configuration::Load()
  {
    std::unique_lock<std::mutex> scope_guard(mtx_accessor);

    if (path.empty() == true)
    {
      return;
    }

    HANDLE file = CreateFileA(path.c_str(), GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

    if (file != INVALID_HANDLE_VALUE)
    {
      FILETIME filetime = {0};
      __time64_t new_last_changed = 0;

      if (TRUE == GetFileTime(file, nullptr, nullptr, &filetime))
      {
        new_last_changed = static_cast<__time64_t>(filetime.dwHighDateTime) << 32 | filetime.dwLowDateTime;
      }

      CloseHandle(file);

      if (new_last_changed == last_changed && last_changed != 0)
      {
        return; // file has not changed.
      }

      last_changed = new_last_changed;
    }

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
    std::unique_lock<std::mutex> scope_guard(mtx_accessor);

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