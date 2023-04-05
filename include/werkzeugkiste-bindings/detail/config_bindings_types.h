#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H

#include <pybind11/chrono.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/config/keymatcher.h>
#include <werkzeugkiste/logging.h>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace werkzeugkiste::bindings::detail {
/// @brief Creates or replaces a configuration parameter.
template <typename TSet, typename TIn>
void GenericSetterUtil(werkzeugkiste::config::Configuration &cfg,
                       std::string_view key, TIn value,
                       const std::string &err_msg) {
  if constexpr (std::is_same_v<TSet, bool>) {
    static_assert(std::is_same_v<TIn, bool>);
    cfg.SetBoolean(key, value);
  } else if constexpr (std::is_arithmetic_v<TSet> &&
                       !std::is_same_v<TSet, bool>) {
    static_assert(std::is_arithmetic_v<TIn>);
    if constexpr (std::is_integral_v<TIn>) {
      cfg.SetInteger64(key, value);
    }
    if constexpr (std::is_floating_point_v<TIn>) {
      cfg.SetDouble(key, value);
    }
    // constexpr auto value_type = std::is_integral_v<TIn>
    //                                 ?
    //                                 werkzeugkiste::config::ConfigType::Integer
    //                                 :
    //                                 werkzeugkiste::config::ConfigType::FloatingPoint;
    // const auto expected = cfg.Contains(key) ? cfg.Type(key) : value_type;

    // if (expected == werkzeugkiste::config::ConfigType::Integer) {
    //   cfg.SetInteger64(key,
    //   werkzeugkiste::config::checked_numcast<int64_t>(value));
    // } else if (expected == werkzeugkiste::config::ConfigType::FloatingPoint)
    // {
    //   cfg.SetDouble(key,
    //   werkzeugkiste::config::checked_numcast<double>(value));
    // } else {
    //   std::string msg{"Changing the type is not allowed. Parameter `"};
    //   msg += key;
    //   msg += "` is `";
    //   msg += werkzeugkiste::config::ConfigTypeToString(expected);
    //   msg += "`, but input value is of type `";
    //   msg += werkzeugkiste::config::TypeName<TIn>();
    //   msg += "!";
    //   throw werkzeugkiste::config::TypeError{msg};
    // }
  } else if constexpr (std::is_same_v<TSet, std::string>) {
    static_assert(std::is_same_v<TIn, std::string>);
    cfg.SetString(key, value);
  } else if constexpr (std::is_same_v<TSet, werkzeugkiste::config::date>) {
    cfg.SetDate(key, value);
  } else if constexpr (std::is_same_v<TSet, werkzeugkiste::config::time>) {
    cfg.SetTime(key, value);
  } else if constexpr (std::is_same_v<TSet, werkzeugkiste::config::date_time>) {
    cfg.SetDateTime(key, value);
  } else {
    throw werkzeugkiste::config::ValueError{err_msg};
  }
}

/// @brief Holds the actual configuration data (to enable shared memory usage
///   among the Config instances).
struct DataHolder {
  werkzeugkiste::config::Configuration data{};
};

class Config {
 public:
  //---------------------------------------------------------------------------
  // Construction / Loading

  static Config LoadFile(pybind11::handle filename) {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data = werkzeugkiste::config::LoadFile(PyObjToString(filename));
    return cfg;
  }

  static Config LoadTOMLFile(pybind11::handle filename) {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data =
        werkzeugkiste::config::LoadTOMLFile(PyObjToString(filename));
    return cfg;
  }

  static Config LoadTOMLString(std::string_view toml_str) {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data = werkzeugkiste::config::LoadTOMLString(toml_str);
    return cfg;
  }

  static Config LoadJSONFile(pybind11::handle filename) {
    // TODO support NullValuePolicy
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data =
        werkzeugkiste::config::LoadJSONFile(PyObjToString(filename));
    return cfg;
  }

  static Config LoadJSONString(std::string_view json_str) {
    // TODO support NullValuePolicy
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data = werkzeugkiste::config::LoadJSONString(json_str);
    return cfg;
  }

  static Config LoadLibconfigFile(pybind11::handle filename) {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data =
        werkzeugkiste::config::LoadLibconfigFile(PyObjToString(filename));
    return cfg;
  }

  static Config LoadLibconfigString(std::string_view lcfg_str) {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data = werkzeugkiste::config::LoadLibconfigString(lcfg_str);
    return cfg;
  }

  static Config FromPyDict(const pybind11::dict &d) {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data = PyDictToConfiguration(d);
    return cfg;
  }

  Config Copy() const {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data = CopyConfig();
    return cfg;
  }

  /// @brief Creates an empty configuration wrapper
  Config() : data_{std::make_shared<DataHolder>()} {}

  //---------------------------------------------------------------------------
  // Serialization

  // TODO fix needed: always returns the full TOML configuration, even for a
  // sub-group view!!! cfg['group'].to_toml() == cfg.to_toml()
  std::string ToTOMLString() const { return ImmutableConfig().ToTOML(); }

  std::string ToJSONString() const { return ImmutableConfig().ToJSON(); }

  std::string ToYAMLString() const { return ImmutableConfig().ToYAML(); }

  std::string ToLibconfigString() const {
    return ImmutableConfig().ToLibconfig();
  }

  pybind11::dict ToDict() const { return GetPyDict(fqn_prefix_); }

  //---------------------------------------------------------------------------
  // Operators/Utils/Basics

  bool Equals(const Config &other) const {
    if (fqn_prefix_.compare(other.fqn_prefix_) == 0) {
      return ImmutableConfig().Equals(other.ImmutableConfig());
    }
    return CopyConfig().Equals(other.CopyConfig());
  }

  bool Contains(std::string_view key) const {
    return ImmutableConfig().Contains(Key(key));
  }

  std::size_t ParameterLength(std::string_view key) const {
    return ImmutableConfig().Size(Key(key));
  }

  inline std::size_t Length() const {
    using namespace std::string_view_literals;
    return ParameterLength(""sv);
  }

  bool Empty() const { return Length() == 0; }

  void Delete(std::string_view key) { MutableConfig().Delete(Key(key)); }

  werkzeugkiste::config::ConfigType ParameterType(std::string_view key) const {
    return ImmutableConfig().Type(Key(key));
  }

  inline werkzeugkiste::config::ConfigType Type() const {
    using namespace std::string_view_literals;
    return ParameterType(""sv);
  }

  //---------------------------------------------------------------------------
  // Getter

  /// @brief Enables `__getitem__[str]` for parameters of type `group`.
  pybind11::object GetKey(std::string_view key,
                          const pybind11::class_<Config> &cls_handle) {
    return Get(Key(key), cls_handle);
  }

  /// @brief Enables `__getitem__[int]` for parameters of type `list`.
  pybind11::object GetIndex(int64_t index,
                            const pybind11::class_<Config> &cls_handle) {
    return Get(Key(index), cls_handle);
  }

  pybind11::object GetBool(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Boolean, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetBoolOr(std::string_view key,
                             const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Boolean, Key(key),
                   /*return_def=*/true, def);
  }

  pybind11::object GetInt(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Integer, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetIntOr(std::string_view key,
                            const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Integer, Key(key),
                   /*return_def=*/true, def);
  }

  pybind11::object GetFloat(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::FloatingPoint, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetFloatOr(std::string_view key,
                              const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::FloatingPoint, Key(key),
                   /*return_def=*/true, def);
  }

  pybind11::object GetStr(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::String, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetStrOr(std::string_view key,
                            const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::String, Key(key),
                   /*return_def=*/true, def);
  }

  pybind11::object GetDate(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Date, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetDateOr(std::string_view key,
                             const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Date, Key(key),
                   /*return_def=*/true, def);
  }

  pybind11::object GetTime(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Time, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetTimeOr(std::string_view key,
                             const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Time, Key(key),
                   /*return_def=*/true, def);
  }

  pybind11::object GetDateTime(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::DateTime, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetDateTimeOr(std::string_view key,
                                 const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::DateTime, Key(key),
                   /*return_def=*/true, def);
  }

  pybind11::object GetList(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::List, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetListOr(std::string_view key,
                             const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::List, Key(key),
                   /*return_def=*/true, def);
  }

  pybind11::object GetDict(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Group, Key(key),
                   /*return_def=*/false);
  }

  pybind11::object GetDictOr(std::string_view key,
                             const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Group, Key(key),
                   /*return_def=*/true, def);
  }

  //---------------------------------------------------------------------------
  // Setter

  /// @brief Enables `__setitem__[str]`.
  void SetKey(std::string_view key, pybind11::handle hnd) {
    Set(Key(key), hnd);
  }

  /// @brief Enables `__setitem__[int]`.
  void SetIndex(int index, pybind11::handle hnd) { Set(Key(index), hnd); }

  //---------------------------------------------------------------------------
  // Special functions

  // TODO make group/list iterable

  std::vector<std::string> ListParameterNames(bool include_array_entries,
                                              std::string_view key) const {
    return ImmutableConfig().ListParameterNames(Key(key), include_array_entries,
                                                /*recursive=*/true);
  }

  std::vector<std::string> Keys() const {
    return ImmutableConfig().ListParameterNames(
        fqn_prefix_, /*include_array_entries=*/false, /*recursive=*/false);
  }

  bool ReplacePlaceholders(
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements,
      std::string_view key) {
    return MutableConfig().ReplaceStringPlaceholders(Key(key), replacements);
  }

  void LoadNested(std::string_view key) {
    MutableConfig().LoadNestedConfiguration(Key(key));
  }

  bool AdjustRelativePaths(pybind11::handle base_path,
                           const std::vector<std::string_view> &parameters,
                           std::string_view key) {
    return MutableConfig().AdjustRelativePaths(
        Key(key), PyObjToString(base_path), parameters);
  }

  // TODO remove
  std::string Debug() const {
    std::string str{"ConfigDemo[prefix '"};
    str += fqn_prefix_;
    str += "', usecnt ";
    str += std::to_string(data_.use_count());
    str += "]";
    return str;
  }

  inline const werkzeugkiste::config::Configuration &ImmutableConfig() const {
    return data_->data;
  }

 private:
  std::shared_ptr<DataHolder> data_{};
  std::string fqn_prefix_{};

  werkzeugkiste::config::Configuration CopyConfig() const {
    if (fqn_prefix_.empty()) {
      return ImmutableConfig();
    }
    return ImmutableConfig().GetGroup(fqn_prefix_);
  }

  inline werkzeugkiste::config::Configuration &MutableConfig() {
    return data_->data;
  }

  inline std::string Key(std::string_view key) const {
    std::string fqn{fqn_prefix_};
    if (!fqn_prefix_.empty() && !key.empty()) {
      fqn += '.';
    }
    fqn += key;
    return fqn;
  }

  inline std::string Key(int index) const {
    const werkzeugkiste::config::ConfigType type = Type();
    if (type == werkzeugkiste::config::ConfigType::List) {
      const int64_t len =
          werkzeugkiste::config::checked_numcast<int64_t>(Length());
      index = (index < 0) ? (len + index) : index;
      std::string fqn{fqn_prefix_};
      fqn += '[';
      fqn += std::to_string(index);
      fqn += ']';
      return fqn;
    }

    std::string msg{
        "Numeric indexing is only supported for list parameters, but `"};
    if (fqn_prefix_.empty()) {
      msg += "<root element>";
    } else {
      msg += fqn_prefix_;
    }
    msg += "` is of type `";
    msg += werkzeugkiste::config::ConfigTypeToString(type);
    msg += "`!";
    throw werkzeugkiste::config::KeyError{msg};
  }

  pybind11::list GetPyList(std::string_view fqn) const {
    const werkzeugkiste::config::Configuration &cfg = ImmutableConfig();
    if (cfg.Type(fqn) != werkzeugkiste::config::ConfigType::List) {
      std::string msg{"Cannot convert parameter `"};
      msg += fqn;
      msg += "` of type `";
      msg += werkzeugkiste::config::ConfigTypeToString(cfg.Type(fqn));
      msg += "` to `list`!";
      throw werkzeugkiste::config::TypeError{msg};
    }

    pybind11::list lst{};
    const std::size_t num_el = cfg.Size(fqn);
    for (std::size_t idx = 0; idx < num_el; ++idx) {
      std::string elem_key{fqn};
      elem_key += '[';
      elem_key += std::to_string(idx);
      elem_key += ']';

      lst.append(ValueOr(cfg.Type(elem_key), elem_key, /*return_def=*/false));
    }
    return lst;
  }

  pybind11::dict GetPyDict(std::string_view fqn) const {
    const werkzeugkiste::config::Configuration &cfg = ImmutableConfig();
    if (!fqn.empty() &&
        (cfg.Type(fqn) != werkzeugkiste::config::ConfigType::Group)) {
      std::string msg{"Cannot convert parameter `"};
      msg += fqn;
      msg += "` of type `";
      msg += werkzeugkiste::config::ConfigTypeToString(cfg.Type(fqn));
      msg += "` to `dict`!";
      throw werkzeugkiste::config::TypeError{msg};
    }

    pybind11::dict d{};
    const std::string cfg_fqn_prefix =
        fqn.empty() ? "" : (std::string(fqn) + '.');
    const std::vector<std::string> keys = cfg.ListParameterNames(
        fqn, /*include_array_entries=*/false, /*recursive=*/false);
    for (const std::string &key : keys) {
      const char *dkey = key.c_str();
      const std::string cfg_key{cfg_fqn_prefix + key};
      d[dkey] = ValueOr(cfg.Type(cfg_key), cfg_key, /*return_def=*/false);
    }
    return d;
  }

  pybind11::object ValueOr(
      werkzeugkiste::config::ConfigType type, std::string_view fqn,
      bool return_def, const pybind11::object &def = pybind11::none()) const {
    const werkzeugkiste::config::Configuration &cfg = ImmutableConfig();
    if (return_def && !cfg.Contains(fqn)) {
      return def;
    }
    // Let wzk throw the KeyError instead (as it will suggest similar
    // parameter names, in case there was a user typo).
    switch (type) {
      case werkzeugkiste::config::ConfigType::Boolean:
        return pybind11::bool_{cfg.GetBoolean(fqn)};

      case werkzeugkiste::config::ConfigType::Integer:
        return pybind11::int_{cfg.GetInteger64(fqn)};

      case werkzeugkiste::config::ConfigType::FloatingPoint:
        return pybind11::float_{cfg.GetDouble(fqn)};

      case werkzeugkiste::config::ConfigType::String:
        return pybind11::str{cfg.GetString(fqn)};

      case werkzeugkiste::config::ConfigType::List:
        return GetPyList(fqn);

      case werkzeugkiste::config::ConfigType::Group:
        return GetPyDict(fqn);

      case werkzeugkiste::config::ConfigType::Date:
        return DateToPyObj(cfg.GetDate(fqn));

      case werkzeugkiste::config::ConfigType::Time:
        return TimeToPyObj(cfg.GetTime(fqn));

      case werkzeugkiste::config::ConfigType::DateTime:
        return DateTimeToPyObj(cfg.GetDateTime(fqn));
    }

    std::string msg{"Returning parameter `"};
    msg += fqn;
    msg += "` as `";
    msg += werkzeugkiste::config::ConfigTypeToString(type);
    msg += "` is not yet implemented!";
    throw std::logic_error{msg};
  }

  pybind11::object Get(std::string_view fqn,
                       const pybind11::class_<Config> &cls_handle) {
    const werkzeugkiste::config::Configuration &cfg = ImmutableConfig();
    const werkzeugkiste::config::ConfigType type = cfg.Type(fqn);

    if ((type == werkzeugkiste::config::ConfigType::List) ||
        (type == werkzeugkiste::config::ConfigType::Group)) {
      pybind11::object obj = cls_handle();
      auto ptr = obj.cast<Config *>();
      ptr->data_ = data_;
      ptr->fqn_prefix_ = fqn;
      return obj;
    }

    return ValueOr(type, fqn, false);
  }

  void Set(std::string_view fqn, pybind11::handle value) {
    werkzeugkiste::config::Configuration &cfg = MutableConfig();
    const std::string py_typestr =
        pybind11::cast<std::string>(value.attr("__class__").attr("__name__"));
    std::optional<werkzeugkiste::config::ConfigType> existing_type{
        std::nullopt};

    std::string err_msg{};
    if (cfg.Contains(fqn)) {
      existing_type = cfg.Type(fqn);
      err_msg = "Cannot use a python object of type `";
      err_msg += py_typestr;
      err_msg += "` to update existing parameter `";
      err_msg += fqn;
      err_msg += "` of type `";
      err_msg +=
          werkzeugkiste::config::ConfigTypeToString(existing_type.value());
      err_msg += "`!";
    } else {
      err_msg = "Cannot create parameter `";
      err_msg += fqn;
      err_msg += "` from python type `";
      err_msg += py_typestr;
      err_msg += "`!";
    }

    // Python type defines what kind of parameter to insert:
    if (pybind11::isinstance<pybind11::str>(value)) {
      GenericSetterUtil<std::string>(cfg, fqn, value.cast<std::string>(),
                                     err_msg);
    } else if (pybind11::isinstance<pybind11::bool_>(value)) {
      GenericSetterUtil<bool>(cfg, fqn, value.cast<bool>(), err_msg);
    } else if (pybind11::isinstance<pybind11::int_>(value)) {
      GenericSetterUtil<int64_t>(cfg, fqn, value.cast<int64_t>(), err_msg);
    } else if (pybind11::isinstance<pybind11::float_>(value)) {
      GenericSetterUtil<double>(cfg, fqn, value.cast<double>(), err_msg);
    } else if (pybind11::isinstance<pybind11::list>(value) ||
               pybind11::isinstance<pybind11::tuple>(value)) {
      ExtractPyIterable(cfg, fqn, value);
    } else if (pybind11::isinstance<pybind11::dict>(value)) {
      cfg.SetGroup(fqn, PyDictToConfiguration(value.cast<pybind11::dict>()));
    } else if (pybind11::isinstance<Config>(value)) {
      cfg.SetGroup(fqn, value.cast<Config>().ImmutableConfig());
    } else {
      if (py_typestr.compare("date") == 0) {
        WZKLOG_CRITICAL("SET {} to date", fqn);
        // GenericScalarSetterUtil<werkzeugkiste::config::date>(self, fqn,
        // value);
        GenericSetterUtil<werkzeugkiste::config::date>(
            cfg, fqn, PyObjToDate(value), err_msg);
      } else if (py_typestr.compare("time") == 0) {
        WZKLOG_CRITICAL("SET {} to time", fqn);
        GenericSetterUtil<werkzeugkiste::config::time>(
            cfg, fqn, PyObjToTime(value), err_msg);
      } else if (py_typestr.compare("datetime") == 0) {
        WZKLOG_CRITICAL("SET {} to datetime", fqn);
        GenericSetterUtil<werkzeugkiste::config::date_time>(
            cfg, fqn, PyObjToDateTime(value), err_msg);
      } else {
        throw werkzeugkiste::config::TypeError{err_msg};
      }
    }
  }
};

inline void ExtractPyIterable(werkzeugkiste::config::Configuration &cfg,
                              std::string_view key, pybind11::handle lst) {
  if (cfg.Contains(key)) {
    cfg.ClearList(key);
  } else {
    cfg.CreateList(key);
  }

  // Invoked with either list or tuple
  for (pybind11::handle value : lst) {
    const std::string tp =
        pybind11::cast<std::string>(value.attr("__class__").attr("__name__"));

    if (pybind11::isinstance<pybind11::str>(value)) {
      cfg.Append(key, value.cast<std::string>());
    } else if (pybind11::isinstance<pybind11::bool_>(value)) {
      cfg.Append(key, value.cast<bool>());
    } else if (pybind11::isinstance<pybind11::int_>(value)) {
      cfg.Append(key, value.cast<int64_t>());
    } else if (pybind11::isinstance<pybind11::float_>(value)) {
      cfg.Append(key, value.cast<double>());
    } else if (pybind11::isinstance<pybind11::list>(value)) {
      std::size_t sz = cfg.Size(key);
      cfg.AppendList(key);
      std::string elem_key{key};
      elem_key += '[';
      elem_key += std::to_string(sz);
      elem_key += ']';
      ExtractPyIterable(cfg, elem_key, value);
    } else if (pybind11::isinstance<Config>(value)) {
      cfg.Append(key, value.cast<Config>().ImmutableConfig());
    } else if (pybind11::isinstance<pybind11::dict>(value)) {
      cfg.Append(key, PyDictToConfiguration(value.cast<pybind11::dict>()));
    } else {
      if (tp.compare("date") == 0) {
        cfg.Append(key, PyObjToDate(value));
      } else if (tp.compare("time") == 0) {
        cfg.Append(key, PyObjToTime(value));
      } else if (tp.compare("datetime") == 0) {
        cfg.Append(key, PyObjToDateTime(value));
      } else {
        std::string msg{"Cannot append a python object of type `"};
        msg += tp;
        msg += "` to list `";
        msg += key;
        msg += "`!";
        throw werkzeugkiste::config::TypeError{msg};
      }
    }
  }
}

inline werkzeugkiste::config::Configuration PyDictToConfiguration(
    const pybind11::dict &d) {
  werkzeugkiste::config::Configuration cfg{};
  for (std::pair<pybind11::handle, pybind11::handle> item : d) {
    if (!pybind11::isinstance<pybind11::str>(item.first)) {
      std::string msg{
          "Dictionary keys must be strings in order to convert to a "
          "configuration parameter, but got `"};
      msg += pybind11::cast<std::string>(
          item.first.attr("__class__").attr("__name__"));
      msg += "`!";
      throw werkzeugkiste::config::TypeError{msg};
    }
    const std::string key = item.first.cast<std::string>();

    if (!werkzeugkiste::config::IsValidKey(key, /*allow_dots=*/false)) {
      std::string msg{"Dictionary key `"};
      msg += key;
      msg +=
          "` is not a valid parameter name! Only alpha-numeric characters, "
          "hyphen and underscore are allowed.";
      throw werkzeugkiste::config::TypeError{msg};
    }

    if (pybind11::isinstance<pybind11::str>(item.second)) {
      cfg.SetString(key, item.second.cast<std::string>());
    } else if (pybind11::isinstance<pybind11::bool_>(item.second)) {
      cfg.SetBoolean(key, item.second.cast<bool>());
    } else if (pybind11::isinstance<pybind11::int_>(item.second)) {
      cfg.SetInteger64(key, item.second.cast<int64_t>());
    } else if (pybind11::isinstance<pybind11::float_>(item.second)) {
      cfg.SetDouble(key, item.second.cast<double>());
    } else if (pybind11::isinstance<pybind11::list>(item.second) ||
               pybind11::isinstance<pybind11::tuple>(item.second)) {
      ExtractPyIterable(cfg, key, item.second);
    } else if (pybind11::isinstance<Config>(item.second)) {
      cfg.SetGroup(key, item.second.cast<Config>().ImmutableConfig());
    } else if (pybind11::isinstance<pybind11::dict>(item.second)) {
      cfg.SetGroup(key,
                   PyDictToConfiguration(item.second.cast<pybind11::dict>()));
    } else {
      const std::string tp = pybind11::cast<std::string>(
          item.second.attr("__class__").attr("__name__"));
      if (tp.compare("date") == 0) {
        cfg.SetDate(key, PyObjToDate(item.second));
      } else if (tp.compare("time") == 0) {
        cfg.SetTime(key, PyObjToTime(item.second));
      } else if (tp.compare("datetime") == 0) {
        cfg.SetDateTime(key, PyObjToDateTime(item.second));
      } else {
        std::string msg{"Cannot convert a python object of type `"};
        msg += tp;
        msg += "` to a configuration parameter. Check dictionary key `";
        msg += key;
        msg += "`!";
        throw werkzeugkiste::config::TypeError{msg};
      }
    }
  }
  return cfg;
}

// class ConfigWrapper {
//  public:
//   //---------------------------------------------------------------------------
//   // Loading / Deserialization

//   static ConfigWrapper LoadFile(pybind11::handle filename) {
//     ConfigWrapper instance{};
//     instance.cfg_ = werkzeugkiste::config::LoadFile(PyObjToString(filename));
//     return instance;
//   }

//   static ConfigWrapper LoadTOMLFile(pybind11::handle filename) {
//     ConfigWrapper instance{};
//     instance.cfg_ =
//     werkzeugkiste::config::LoadTOMLFile(PyObjToString(filename)); return
//     instance;
//   }

//   static ConfigWrapper LoadTOMLString(std::string_view toml_str) {
//     ConfigWrapper instance{};
//     instance.cfg_ = werkzeugkiste::config::LoadTOMLString(toml_str);
//     return instance;
//   }

//   static ConfigWrapper LoadJSONFile(pybind11::handle filename) {
//     // TODO support NullValuePolicy
//     ConfigWrapper instance{};
//     instance.cfg_ =
//     werkzeugkiste::config::LoadJSONFile(PyObjToString(filename)); return
//     instance;
//   }

//   static ConfigWrapper LoadJSONString(std::string_view json_str) {
//     // TODO support NullValuePolicy
//     ConfigWrapper instance{};
//     instance.cfg_ = werkzeugkiste::config::LoadJSONString(json_str);
//     return instance;
//   }

//   static ConfigWrapper LoadLibconfigFile(pybind11::handle filename) {
//     ConfigWrapper instance{};
//     instance.cfg_ =
//     werkzeugkiste::config::LoadLibconfigFile(PyObjToString(filename)); return
//     instance;
//   }

//   static ConfigWrapper LoadLibconfigString(std::string_view lcfg_str) {
//     ConfigWrapper instance{};
//     instance.cfg_ = werkzeugkiste::config::LoadLibconfigString(lcfg_str);
//     return instance;
//   }

//   static ConfigWrapper FromPyDict(const pybind11::dict &d) {
//     ConfigWrapper instance{};
//     for (std::pair<pybind11::handle, pybind11::handle> item : d) {
//       if (!pybind11::isinstance<pybind11::str>(item.first)) {
//         std::string msg{"Dictionary keys must be strings, but got `"};
//         msg +=
//         pybind11::cast<std::string>(item.first.attr("__class__").attr("__name__"));
//         msg += "`!";
//         throw werkzeugkiste::config::TypeError{msg};
//       }
//       const std::string key = item.first.cast<std::string>();

//       if (!werkzeugkiste::config::IsValidKey(key, /*allow_dots=*/false)) {
//         std::string msg{"Dictionary key `"};
//         msg += key;
//         msg += "` is not a valid parameter name! Only alpha-numeric
//         characters, hyphen and underscore are allowed."; throw
//         werkzeugkiste::config::TypeError{msg};
//       }

//       if (pybind11::isinstance<pybind11::str>(item.second)) {
//         instance.SetString(key, item.second.cast<std::string>());
//       } else if (pybind11::isinstance<pybind11::bool_>(item.second)) {
//         instance.SetBoolean(key, item.second.cast<bool>());
//       } else if (pybind11::isinstance<pybind11::int_>(item.second)) {
//         instance.SetInteger64(key, item.second.cast<int64_t>());
//       } else if (pybind11::isinstance<pybind11::float_>(item.second)) {
//         instance.SetDouble(key, item.second.cast<double>());
//       } else if (pybind11::isinstance<pybind11::list>(item.second)) {
//         instance.SetList(key, item.second.cast<pybind11::list>());
//       } else if (pybind11::isinstance<ConfigWrapper>(item.second)) {
//         instance.SetGroup(key, item.second.cast<ConfigWrapper>());
//       } else if (pybind11::isinstance<pybind11::dict>(item.second)) {
//         instance.SetGroup(key,
//         FromPyDict(item.second.cast<pybind11::dict>()));
//       } else {
//         const std::string tp =
//           pybind11::cast<std::string>(item.second.attr("__class__").attr("__name__"));
//         if (tp.compare("date") == 0) {
//           instance.SetDate(key, item.second);
//         } else if (tp.compare("time") == 0) {
//           instance.SetTime(key, item.second);
//         } else if (tp.compare("datetime") == 0) {
//           instance.SetDateTime(key, item.second);
//         } else {
//           std::string msg{"Cannot convert a python object of type `"};
//           msg += tp;
//           msg += "` to a configuration parameter. Check dictionary key `";
//           msg += key;
//           msg += "`!";
//           throw werkzeugkiste::config::TypeError{msg};
//         }
//       }
//     }
//     return instance;
//   }

//   //---------------------------------------------------------------------------
//   // Serialization

//   std::string ToTOMLString() const { return cfg_.ToTOML(); }

//   std::string ToJSONString() const { return cfg_.ToJSON(); }

//   std::string ToYAMLString() const { return cfg_.ToYAML(); }

//   std::string ToLibconfigString() const { return cfg_.ToLibconfig(); }

//   pybind11::object ToDict() const {
//     return GroupToPyObj(nullptr, cfg_);
//   }

//   //---------------------------------------------------------------------------
//   // Operators/Utils/Basics

//   bool Equals(const ConfigWrapper &other) const {
//     return cfg_.Equals(other.cfg_);
//   }

//   bool Empty() const { return cfg_.Empty(); }

//   bool Contains(std::string_view key) const { return cfg_.Contains(key); }

//   std::size_t Size() const { return cfg_.Size(); }

//   std::size_t ParameterSize(std::string_view key) const {
//     return cfg_.Size(key);
//   }

//   werkzeugkiste::config::ConfigType Type(std::string_view key) const { return
//   cfg_.Type(key); }

//   void ReplaceConfig(werkzeugkiste::config::Configuration &&c) { cfg_ =
//   std::move(c); } void ReplaceConfig(const
//   werkzeugkiste::config::Configuration &c) { cfg_ = c; }

//   void Delete(std::string_view key) {
//     cfg_.Delete(key);
//   }

//   //
//   //---------------------------------------------------------------------------
//   // // Generic access
//   // ParameterView Get(std::string_view key) {
//   //   // Non-const to support chained __getitem__/__setitem__ call chains
//   //   return ParameterView{&cfg_, key};
//   // }

//   // void Set(std::string_view key, pybind11::handle obj) {
//   //   //TODO
//   // }

//   //---------------------------------------------------------------------------
//   // Boolean

//   void SetBoolean(std::string_view key, bool value) {
//     cfg_.SetBoolean(key, value);
//   }

//   bool GetBoolean(std::string_view key) const { return cfg_.GetBoolean(key);
//   }

//   bool GetBooleanOr(std::string_view key, bool default_value) const {
//     return cfg_.GetBooleanOr(key, default_value);
//   }

//   //---------------------------------------------------------------------------
//   // Integer

//   void SetInteger64(std::string_view key, int64_t value) {
//     cfg_.SetInteger64(key, value);
//   }

//   int64_t GetInteger64(std::string_view key) const {
//     return cfg_.GetInteger64(key);
//   }

//   int64_t GetInteger64Or(std::string_view key, int64_t default_value) const {
//     return cfg_.GetInteger64Or(key, default_value);
//   }

//   //---------------------------------------------------------------------------
//   // Floating point

//   void SetDouble(std::string_view key, double value) {
//     cfg_.SetDouble(key, value);
//   }

//   double GetDouble(std::string_view key) const { return cfg_.GetDouble(key);
//   }

//   double GetDoubleOr(std::string_view key, double default_value) const {
//     return cfg_.GetDoubleOr(key, default_value);
//   }

//   //---------------------------------------------------------------------------
//   // Strings

//   void SetString(std::string_view key, std::string_view value) {
//     cfg_.SetString(key, value);
//   }

//   std::string GetString(std::string_view key) const {
//     return cfg_.GetString(key);
//   }

//   std::string GetStringOr(std::string_view key,
//                           std::string_view default_value) const {
//     return cfg_.GetStringOr(key, default_value);
//   }

//   //---------------------------------------------------------------------------
//   // Date

//   void SetDate(std::string_view key, pybind11::handle value) {
//     const werkzeugkiste::config::date d = PyObjToDate(value);
//     cfg_.SetDate(key, d);
//   }

//   pybind11::object GetDate(std::string_view key) const {
//     return DateToPyObj(cfg_.GetDate(key));
//   }

//   pybind11::object GetDateOr(std::string_view key,
//                              pybind11::handle default_value) const {
//     const werkzeugkiste::config::date d = PyObjToDate(default_value);
//     return DateToPyObj(cfg_.GetDateOr(key, d));
//   }

//   //---------------------------------------------------------------------------
//   // Time

//   void SetTime(std::string_view key, pybind11::handle value) {
//     const werkzeugkiste::config::time t = PyObjToTime(value);
//     cfg_.SetTime(key, t);
//   }

//   pybind11::object GetTime(std::string_view key) const {
//     return TimeToPyObj(cfg_.GetTime(key));
//   }

//   pybind11::object GetTimeOr(std::string_view key,
//                              pybind11::handle default_value) const {
//     const werkzeugkiste::config::time t = PyObjToTime(default_value);
//     return TimeToPyObj(cfg_.GetTimeOr(key, t));
//   }

//   //---------------------------------------------------------------------------
//   // DateTime

//   void SetDateTime(std::string_view key, pybind11::handle value) {
//     const werkzeugkiste::config::date_time dt = PyObjToDateTime(value);
//     cfg_.SetDateTime(key, dt);
//   }

//   pybind11::object GetDateTime(std::string_view key) const {
//     return DateTimeToPyObj(cfg_.GetDateTime(key));
//   }

//   pybind11::object GetDateTimeOr(std::string_view key,
//                                  pybind11::handle default_value) const {
//     const werkzeugkiste::config::date_time dt =
//     PyObjToDateTime(default_value); return
//     DateTimeToPyObj(cfg_.GetDateTimeOr(key, dt));
//   }

//   //---------------------------------------------------------------------------
//   // Group
//   void SetGroup(std::string_view key, const ConfigWrapper &cfg) {
//     cfg_.SetGroup(key, cfg.cfg_);
//   }

//   werkzeugkiste::config::Configuration GetGroup(std::string_view key) const {
//     return cfg_.GetGroup(key);
//   }

//   const werkzeugkiste::config::Configuration &GetRootGroup() const {
//     return cfg_;
//   }

//   //---------------------------------------------------------------------------
//   // List
//   void SetList(std::string_view key, const pybind11::object &lst) {
//     if (cfg_.Contains(key)) {
//       cfg_.ClearList(key);
//     } else {
//       cfg_.CreateList(key);
//     }

//     // Invoked with either list or tuple
//     for (pybind11::handle value : lst) {
//       const std::string tp =
//           pybind11::cast<std::string>(value.attr("__class__").attr("__name__"));

//       if (pybind11::isinstance<pybind11::str>(value)) {
//         cfg_.Append(key, value.cast<std::string>());
//       } else if (pybind11::isinstance<pybind11::bool_>(value)) {
//         cfg_.Append(key, value.cast<bool>());
//       } else if (pybind11::isinstance<pybind11::int_>(value)) {
//         cfg_.Append(key, value.cast<int64_t>());
//       } else if (pybind11::isinstance<pybind11::float_>(value)) {
//         cfg_.Append(key, value.cast<double>());
//       } else if (pybind11::isinstance<pybind11::list>(value)) {
//         std::size_t sz = cfg_.Size(key);
//         cfg_.AppendList(key);
//         std::string elem_key{key};
//         elem_key += '[';
//         elem_key += std::to_string(sz);
//         elem_key += ']';
//         SetList(elem_key, value.cast<pybind11::list>());
//       } else if (pybind11::isinstance<ConfigWrapper>(value)) {
//         cfg_.Append(key, value.cast<ConfigWrapper>().cfg_);
//       } else if (pybind11::isinstance<pybind11::dict>(value)) {
//         cfg_.Append(key, FromPyDict(value.cast<pybind11::dict>()).cfg_);
//       } else {
//         if (tp.compare("date") == 0) {
//           cfg_.Append(key, PyObjToDate(value));
//         } else if (tp.compare("time") == 0) {
//           cfg_.Append(key, PyObjToTime(value));
//         } else if (tp.compare("datetime") == 0) {
//           cfg_.Append(key, PyObjToDateTime(value));
//         } else {
//           std::string msg{"Cannot append a python object of type `"};
//           msg += tp;
//           msg += "` to list `";
//           msg += key;
//           msg += "`!";
//           throw werkzeugkiste::config::TypeError{msg};
//         }
//       }
//     }
//   }

//   //---------------------------------------------------------------------------
//   // Special functions
//   std::vector<std::string> ListParameterNames(
//       bool include_array_entries) const {
//     return cfg_.ListParameterNames(include_array_entries, true);
//   }

//   std::vector<std::string> Keys() const {
//     return cfg_.ListParameterNames(false, false);
//   }

//   bool ReplacePlaceholders(
//       const std::vector<std::pair<std::string_view, std::string_view>>
//           &replacements) {
//     return cfg_.ReplaceStringPlaceholders(replacements);
//   }

//   void LoadNested(std::string_view key) { cfg_.LoadNestedConfiguration(key);
//   }

//   bool AdjustRelativePaths(pybind11::handle base_path,
//                            const std::vector<std::string_view> &parameters) {
//     return cfg_.AdjustRelativePaths(PyObjToString(base_path), parameters);
//   }

//  private:
//   ConfigWrapper *root_view_{nullptr};
//   werkzeugkiste::config::Configuration cfg_{};

//   inline const werkzeugkiste::config::Configuration &ImmutableConfig() const
//   {
//     if (root_view_) {
//       return root_view_->ImmutableConfig();
//     }
//     return cfg_;
//   }

//   inline werkzeugkiste::config::Configuration &MutableConfig() {
//     if (root_view_) {
//       return root_view_->MutableConfig();
//     }
//     return cfg_;
//   }
// };

inline werkzeugkiste::config::date PyObjToDateUnchecked(pybind11::handle obj) {
  const int year = obj.attr("year").cast<int>();
  const int month = obj.attr("month").cast<int>();
  const int day = obj.attr("day").cast<int>();
  return werkzeugkiste::config::date{
      werkzeugkiste::config::checked_numcast<uint32_t>(year),
      werkzeugkiste::config::checked_numcast<uint32_t>(month),
      werkzeugkiste::config::checked_numcast<uint32_t>(day)};
}

inline werkzeugkiste::config::time PyObjToTimeUnchecked(pybind11::handle obj) {
  const int hour = obj.attr("hour").cast<int>();
  const int minute = obj.attr("minute").cast<int>();
  const int second = obj.attr("second").cast<int>();
  const int microsec = obj.attr("microsecond").cast<int>();
  return werkzeugkiste::config::time{
      werkzeugkiste::config::checked_numcast<uint32_t>(hour),
      werkzeugkiste::config::checked_numcast<uint32_t>(minute),
      werkzeugkiste::config::checked_numcast<uint32_t>(second),
      werkzeugkiste::config::checked_numcast<uint32_t>(microsec * 1000)};
}

inline werkzeugkiste::config::date PyObjToDate(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return werkzeugkiste::config::date{obj.cast<std::string>()};
  }

  // We need to ensure that the PyDateTime import is initialized.
  // Or prepare for segfaults.
  if (!PyDateTimeAPI) {
    PyDateTime_IMPORT;
  }

  if (PyDate_CheckExact(obj.ptr())) {  // || PyDateTime_Check(obj.ptr())) {
    // Object is datetime.date
    return PyObjToDateUnchecked(obj);
  }

  const std::string tp =
      pybind11::cast<std::string>(obj.attr("__class__").attr("__name__"));
  std::string msg{"Cannot convert python type `"};
  msg += tp;
  msg += "` to `werkzeugkiste::date`!";
  throw werkzeugkiste::config::TypeError(msg);
}

inline werkzeugkiste::config::time PyObjToTime(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return werkzeugkiste::config::time{obj.cast<std::string>()};
  }

  // We need to ensure that the PyDateTime import is initialized.
  // Or prepare for segfaults.
  if (!PyDateTimeAPI) {
    PyDateTime_IMPORT;
  }

  // Currently, only conversion from a datetime.time object is allowed. An
  // implicit conversion from a PyDateTime object would be convenient, but
  // also a pain to debug. Consider for example:
  //   now = datetime.datetime.now()
  //   cfg['did_not_exist'] = now  # --> will be datetime.datetime
  //   cfg['key'] = now  # Does the user want to replace an existing 'key' of
  //                     # type `time` by `now.time()` or change the type to
  //                     # datetime.datetime?
  if (PyTime_CheckExact(obj.ptr())) {
    // Object is datetime.time
    return PyObjToTimeUnchecked(obj);
  }

  const std::string tp =
      pybind11::cast<std::string>(obj.attr("__class__").attr("__name__"));
  std::string msg{"Cannot convert python type `"};
  msg += tp;
  msg += "` to `werkzeugkiste::time`!";
  throw werkzeugkiste::config::TypeError(msg);
}

inline werkzeugkiste::config::date_time PyObjToDateTime(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return werkzeugkiste::config::date_time{obj.cast<std::string>()};
  }

  // We need to ensure that the PyDateTime import is initialized.
  // Or prepare for segfaults.
  if (!PyDateTimeAPI) {
    PyDateTime_IMPORT;
  }

  if (PyDateTime_Check(obj.ptr())) {
    // Object is datetime.datetime or subtype
    const werkzeugkiste::config::date d = PyObjToDateUnchecked(obj);
    const werkzeugkiste::config::time t = PyObjToTimeUnchecked(obj);

    if (obj.attr("tzinfo").is_none()) {
      return werkzeugkiste::config::date_time{d, t};
    } else {
      const auto pyoffset_sec = obj.attr("utcoffset")().attr("total_seconds")();
      const auto offset_min =
          static_cast<int32_t>(pyoffset_sec.cast<double>() / 60.0);
      return werkzeugkiste::config::date_time{
          d, t, werkzeugkiste::config::time_offset{offset_min}};
    }
  }

  const std::string tp =
      pybind11::cast<std::string>(obj.attr("__class__").attr("__name__"));
  std::string msg{"Cannot convert python type `"};
  msg += tp;
  msg += "` to `werkzeugkiste::date_time`!";
  throw werkzeugkiste::config::TypeError(msg);
}

inline pybind11::object DateToPyObj(const werkzeugkiste::config::date &d) {
  auto pydatetime = pybind11::module::import("datetime");
  return pydatetime.attr("date")(d.year, d.month, d.day);
}

inline pybind11::object TimeToPyObj(const werkzeugkiste::config::time &t) {
  auto pydatetime = pybind11::module::import("datetime");
  return pydatetime.attr("time")(t.hour, t.minute, t.second,
                                 t.nanosecond / 1000);
}

inline pybind11::object DateTimeToPyObj(
    const werkzeugkiste::config::date_time &dt) {
  auto pydatetime = pybind11::module::import("datetime");
  if (dt.IsLocal()) {
    return pydatetime.attr("datetime")(
        dt.date.year, dt.date.month, dt.date.day, dt.time.hour, dt.time.minute,
        dt.time.second, dt.time.nanosecond / 1000);
  }

  auto timedelta = pydatetime.attr("timedelta");
  auto timezone = pydatetime.attr("timezone");
  // Ctor parameter order is: (days=0, seconds=0, microseconds=0,
  //   milliseconds=0, minutes=0, hours=0, weeks=0)
  // https://docs.python.org/3/library/datetime.html#datetime.timedelta
  auto offset_td = timedelta(0, 0, 0, 0, dt.offset.value().minutes);
  return pydatetime.attr("datetime")(
      dt.date.year, dt.date.month, dt.date.day, dt.time.hour, dt.time.minute,
      dt.time.second, dt.time.nanosecond / 1000, timezone(offset_td));
}

// // TODO remove
// /// If the class handle is provided, the configuration group will be returned
// /// as a ConfigWrapper instance (which is available in python).
// /// Otherwise, the configuration group will be returned as a pure python
// dict. inline pybind11::object GroupToPyObj(
//     const pybind11::class_<ConfigWrapper> *cls_handle,
//     const werkzeugkiste::config::Configuration &cfg) {
//   if (cls_handle) {
//     pybind11::object obj = (*cls_handle)();
//     auto *ptr = obj.cast<ConfigWrapper *>();
//     ptr->ReplaceConfig(cfg);
//     return obj;
//   }

//   pybind11::dict d;
//   const std::vector<std::string> keys = cfg.ListParameterNames(false, false);
//   for (const std::string &key : keys) {
//     const char *dkey = key.c_str();
//     switch (cfg.Type(key)) {
//       case werkzeugkiste::config::ConfigType::Boolean:
//         d[dkey] = cfg.GetBoolean(key);
//         break;

//       case werkzeugkiste::config::ConfigType::Integer:
//         d[dkey] = cfg.GetInteger64(key);
//         break;

//       case werkzeugkiste::config::ConfigType::FloatingPoint:
//         d[dkey] = cfg.GetDouble(key);
//         break;

//       case werkzeugkiste::config::ConfigType::String:
//         d[dkey] = cfg.GetString(key);
//         break;

//       case werkzeugkiste::config::ConfigType::Date:
//         d[dkey] = DateToPyObj(cfg.GetDate(key));
//         break;

//       case werkzeugkiste::config::ConfigType::Time:
//         d[dkey] = TimeToPyObj(cfg.GetTime(key));
//         break;

//       case werkzeugkiste::config::ConfigType::DateTime:
//         d[dkey] = DateTimeToPyObj(cfg.GetDateTime(key));
//         break;

//       case werkzeugkiste::config::ConfigType::List:
//         d[dkey] = ListToPyList(cls_handle, cfg, key);
//         break;

//       case werkzeugkiste::config::ConfigType::Group:
//         d[dkey] = GroupToPyObj(cls_handle, cfg.GetGroup(key));
//         break;
//     }
//   }
//   return d;
// }

// // TODO remove
// inline pybind11::list ListToPyList(const pybind11::class_<ConfigWrapper>
// *cls_handle,
//                                    const werkzeugkiste::config::Configuration
//                                    &cfg, std::string_view key) {
//   // TODO nice-to-have: create a custom readonly_list, so users can't run
//   into
//   // bugs like: cfg['lst'][3] = 'this will NOT change the parameter!'

//   pybind11::list lst{};
//   const std::size_t num_el = cfg.Size(key);

//   for (std::size_t idx = 0; idx < num_el; ++idx) {
//     std::string elem_key{key};
//     elem_key += '[';
//     elem_key += std::to_string(idx);
//     elem_key += ']';

//     switch (cfg.Type(elem_key)) {
//       case werkzeugkiste::config::ConfigType::Boolean:
//         lst.append(cfg.GetBoolean(elem_key));
//         break;

//       case werkzeugkiste::config::ConfigType::Integer:
//         lst.append(cfg.GetInteger64(elem_key));
//         break;

//       case werkzeugkiste::config::ConfigType::FloatingPoint:
//         lst.append(cfg.GetDouble(elem_key));
//         break;

//       case werkzeugkiste::config::ConfigType::String:
//         lst.append(cfg.GetString(elem_key));
//         break;

//       case werkzeugkiste::config::ConfigType::List:
//         lst.append(ListToPyList(cls_handle, cfg, elem_key));
//         break;

//       case werkzeugkiste::config::ConfigType::Group:
//         lst.append(GroupToPyObj(cls_handle, cfg.GetGroup(elem_key)));
//         break;

//       case werkzeugkiste::config::ConfigType::Date:
//         lst.append(DateToPyObj(cfg.GetDate(elem_key)));
//         break;

//       case werkzeugkiste::config::ConfigType::Time:
//         lst.append(TimeToPyObj(cfg.GetTime(elem_key)));
//         break;

//       case werkzeugkiste::config::ConfigType::DateTime:
//         lst.append(DateTimeToPyObj(cfg.GetDateTime(elem_key)));
//     }
//   }
//   return lst;
// }

inline std::string PyObjToString(pybind11::handle hnd) {
  if (pybind11::isinstance<pybind11::str>(hnd)) {
    return hnd.cast<std::string>();
  } else {
    return pybind11::str(hnd).cast<std::string>();
  }
}

}  // namespace werkzeugkiste::bindings::detail

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H
