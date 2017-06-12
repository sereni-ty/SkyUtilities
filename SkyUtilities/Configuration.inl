template<typename T>
bool Configuration::Get(const Configuration::Setting<T> &setting, T &value)
{
  auto entry_it = entries.find(setting.key);

  if (entry_it == entries.end())
  {
    value = setting.default_value;
    return false;
  }

  (*entry_it).second >> value;
  (*entry_it).second.seekg(0);

  value = setting.value_limits_check(value);

  return true;
}

template<typename T>
void Configuration::Set(const Setting<T> &setting, const T &value)
{
  std::stringstream ss;
  ss << value;

  entries.try_emplace(setting.key, std::move(ss));

  Save();
}

template<typename T>
void Configuration::SetInitial(const Setting<T> &setting)
{
  if (entries.find(setting.key) != entries.end())
  {
    return;
  }

  Set<T>(setting, setting.default_value);
}