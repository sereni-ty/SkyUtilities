template<typename T>
bool Configuration::Get(const Configuration::Setting<T> &setting, T &value)
{
  boost::optional<T> json_value = json_values.get_optional<T>(setting.key);

  if (json_value.is_initialized() == false)
  {
    value = setting.default_value;
    return false;
  }

  value = setting.value_limits_check(json_value.get());

  return true;
}

template<typename T>
void Configuration::Set(const Setting<T> &setting, const T &value)
{
  json_values.put<T>(setting.key, value);

  Save();
}

template<typename T>
void Configuration::SetInitial(const Setting<T> &setting)
{
  if (json_values.find(setting.key) != json_values.not_found())
  {
    return;
  }

  Set<T>(setting, setting.default_value);
}