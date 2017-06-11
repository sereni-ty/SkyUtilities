template<typename T>
bool Configuration::Get(const std::string &key, T &value)
{
  auto entry_it = entries.find(key);

  if (entry_it == entries.end())
  {
    return false;
  }

  *entry_it >> value;
  return true;
}

template<typename T>
bool Configuration::Get(const std::string &key, T &value, std::function<T(const T&)> value_limits_check)
{
  if (Get(key, value) == false)
  {
    return false;
  }

  value = value_limits_check(value);

  return true;
}

template<typename T>
void Configuration::Set(const std::string &key, const T &value)
{
  std::stringstream ss;
  ss << value;

  entries.try_emplace(key, std::move(ss));

  Save();
}

template<typename T>
void Configuration::SetInitial(const std::string &key, const T &value)
{
  if (entries.find(key) != entries.end())
  {
    return;
  }

  std::stringstream ss;
  ss << value;

  entries.try_emplace(key, std::move(ss));

  Save();
}