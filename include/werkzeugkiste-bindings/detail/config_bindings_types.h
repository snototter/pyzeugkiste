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
/// @brief Copies the list `fqn_src` from `src` to `fqn_dst` in `dst`.
inline void CopyList(const werkzeugkiste::config::Configuration &src,
    std::string_view fqn_src,
    werkzeugkiste::config::Configuration &dst,
    std::string_view fqn_dst) {
  if (!dst.Contains(fqn_dst)) {
    std::string msg{"CopyList requires that the list parameter `"};
    msg += fqn_dst;
    msg += "` already exists!";
    throw std::logic_error{msg};
  }

  const std::size_t size_src = src.Size(fqn_src);
  for (std::size_t idx = 0; idx < size_src; ++idx) {
    std::string fqn_src_elem{fqn_src};
    fqn_src_elem += '[';
    fqn_src_elem += std::to_string(idx);
    fqn_src_elem += ']';

    switch (src.Type(fqn_src_elem)) {
      case werkzeugkiste::config::ConfigType::Boolean:
        dst.Append(fqn_dst, src.GetBoolean(fqn_src_elem));
        break;

      case werkzeugkiste::config::ConfigType::Integer:
        dst.Append(fqn_dst, src.GetInteger64(fqn_src_elem));
        break;

      case werkzeugkiste::config::ConfigType::FloatingPoint:
        dst.Append(fqn_dst, src.GetDouble(fqn_src_elem));
        break;

      case werkzeugkiste::config::ConfigType::String:
        dst.Append(fqn_dst, src.GetString(fqn_src_elem));
        break;

      case werkzeugkiste::config::ConfigType::List: {
        // We need to append a list, then recurse with a
        // properly adjusted key
        std::size_t size_dst = dst.Size(fqn_dst);
        dst.AppendList(fqn_dst);
        std::string fqn_dst_elem{fqn_dst};
        fqn_dst_elem += '[';
        fqn_dst_elem += std::to_string(size_dst);
        fqn_dst_elem += ']';
        dst.AppendList(fqn_dst);
        CopyList(src, fqn_src_elem, dst, fqn_dst_elem);
        break;
      }

      case werkzeugkiste::config::ConfigType::Group:
        dst.Append(fqn_dst, src.GetGroup(fqn_src_elem));
        break;

      case werkzeugkiste::config::ConfigType::Date:
        dst.Append(fqn_dst, src.GetDate(fqn_src_elem));
        break;

      case werkzeugkiste::config::ConfigType::Time:
        dst.Append(fqn_dst, src.GetTime(fqn_src_elem));
        break;

      case werkzeugkiste::config::ConfigType::DateTime:
        dst.Append(fqn_dst, src.GetDateTime(fqn_src_elem));
        break;
    }
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
    if (Type() != werkzeugkiste::config::ConfigType::Group) {
      std::string msg{
          "Cannot create a deep copy of a configuration view on a `"};
      msg += werkzeugkiste::config::ConfigTypeToString(Type());
      msg += "`. Only (sub-)groups can be copied!";
      throw werkzeugkiste::config::TypeError{msg};
    }

    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data = CopyViewedGroup();
    return cfg;
  }

  /// @brief Creates an empty configuration wrapper
  Config() : data_{std::make_shared<DataHolder>()} {}

  //---------------------------------------------------------------------------
  // Serialization

  std::string ToTOMLString() const { return CopyViewedGroup().ToTOML(); }

  std::string ToJSONString() const { return CopyViewedGroup().ToJSON(); }

  std::string ToYAMLString() const { return CopyViewedGroup().ToYAML(); }

  std::string ToLibconfigString() const {
    return CopyViewedGroup().ToLibconfig();
  }

  pybind11::dict ToDict() const { return GetPyDict(fqn_prefix_); }

  //---------------------------------------------------------------------------
  // Operators/Utils/Basics

  bool Equals(const Config &other) const {
    return CopyViewedGroup().Equals(other.CopyViewedGroup());
  }

  bool Contains(std::string_view key) const {
    // TODO could add a check to prevent invocation on a List "view"
    // https://docs.python.org/3/reference/datamodel.html?emulating-container-types#emulating-container-types
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
    if (fqn_prefix_.empty()) {
      return werkzeugkiste::config::ConfigType::Group;
    }

    using namespace std::string_view_literals;
    return ParameterType(""sv);
  }

  void Clear() {
    if (Type() == werkzeugkiste::config::ConfigType::List) {
      MutableConfig().ClearList(fqn_prefix_);
    } else {
      for (const auto &key : Keys()) {
        Delete(key);
      }
    }
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
    return ValueOr(werkzeugkiste::config::ConfigType::Boolean,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetBoolOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Boolean,
        Key(key),
        /*return_def=*/true,
        def);
  }

  pybind11::object GetInt(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Integer,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetIntOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Integer,
        Key(key),
        /*return_def=*/true,
        def);
  }

  pybind11::object GetFloat(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::FloatingPoint,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetFloatOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::FloatingPoint,
        Key(key),
        /*return_def=*/true,
        def);
  }

  pybind11::object GetStr(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::String,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetStrOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::String,
        Key(key),
        /*return_def=*/true,
        def);
  }

  pybind11::object GetDate(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Date,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetDateOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Date,
        Key(key),
        /*return_def=*/true,
        def);
  }

  pybind11::object GetTime(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Time,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetTimeOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Time,
        Key(key),
        /*return_def=*/true,
        def);
  }

  pybind11::object GetDateTime(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::DateTime,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetDateTimeOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::DateTime,
        Key(key),
        /*return_def=*/true,
        def);
  }

  pybind11::object GetList(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::List,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetListOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::List,
        Key(key),
        /*return_def=*/true,
        def);
  }

  pybind11::object GetDict(std::string_view key) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Group,
        Key(key),
        /*return_def=*/false);
  }

  pybind11::object GetDictOr(std::string_view key,
      const pybind11::object &def) const {
    return ValueOr(werkzeugkiste::config::ConfigType::Group,
        Key(key),
        /*return_def=*/true,
        def);
  }

  //---------------------------------------------------------------------------
  // Setter

  /// @brief Enables `__setitem__[str]`.
  void SetKey(std::string_view key, pybind11::handle hnd) {
    Set(Key(key), hnd);
  }

  /// @brief Enables `__setitem__[int]`.
  void SetIndex(int index, pybind11::handle hnd) { Set(Key(index), hnd); }

  /// @brief Appends an object to a list, optionally creates it.
  void Append(std::string_view key, pybind11::handle hnd) {
    AppendToList(Key(key), hnd);
  }

  /// @brief Allows `append(obj)` for a "list view".
  inline void Append(pybind11::handle hnd) {
    using namespace std::string_view_literals;
    Append(""sv, hnd);
  }

  //---------------------------------------------------------------------------
  // Special functions

  // TODO make group/list iterable

  std::vector<std::string> ListParameterNames(bool include_array_entries,
      bool recursive,
      std::string_view key) const {
    return ImmutableConfig().ListParameterNames(
        Key(key), include_array_entries, recursive);
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

  inline const werkzeugkiste::config::Configuration &ImmutableConfig() const {
    return data_->data;
  }

 private:
  std::shared_ptr<DataHolder> data_{};
  std::string fqn_prefix_{};

  werkzeugkiste::config::Configuration ListToGroup() const {
    using namespace std::string_view_literals;
    werkzeugkiste::config::Configuration copy{};
    const werkzeugkiste::config::Configuration &cfg = ImmutableConfig();
    std::string_view key_out{"list"sv};
    copy.CreateList(key_out);
    for (std::size_t idx = 0; idx < Length(); ++idx) {
      const std::string fqn = Key(idx);
      switch (cfg.Type(fqn)) {
        case werkzeugkiste::config::ConfigType::Boolean:
          copy.Append(key_out, cfg.GetBoolean(fqn));
          break;

        case werkzeugkiste::config::ConfigType::Integer:
          copy.Append(key_out, cfg.GetInteger64(fqn));
          break;

        case werkzeugkiste::config::ConfigType::FloatingPoint:
          copy.Append(key_out, cfg.GetDouble(fqn));
          break;

        case werkzeugkiste::config::ConfigType::String:
          copy.Append(key_out, cfg.GetString(fqn));
          break;

        case werkzeugkiste::config::ConfigType::List:
          // copy.Append(key_out, cfg.GetBoolean(fqn)); FIXME
          break;

        case werkzeugkiste::config::ConfigType::Group:
          copy.Append(key_out, cfg.GetGroup(fqn));
          break;

        case werkzeugkiste::config::ConfigType::Date:
          copy.Append(key_out, cfg.GetDate(fqn));
          break;

        case werkzeugkiste::config::ConfigType::Time:
          copy.Append(key_out, cfg.GetTime(fqn));
          break;

        case werkzeugkiste::config::ConfigType::DateTime:
          copy.Append(key_out, cfg.GetDateTime(fqn));
          break;
      }
    }
    return copy;
  }

  werkzeugkiste::config::Configuration CopyViewedGroup() const {
    if (fqn_prefix_.empty()) {
      return ImmutableConfig();
    }
    if (Type() == werkzeugkiste::config::ConfigType::List) {
      using namespace std::string_view_literals;
      werkzeugkiste::config::Configuration cfg{};
      cfg.CreateList("list"sv);
      CopyList(ImmutableConfig(), fqn_prefix_, cfg, "list"sv);
      return cfg;
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

  pybind11::object ValueOr(werkzeugkiste::config::ConfigType type,
      std::string_view fqn,
      bool return_def,
      const pybind11::object &def = pybind11::none()) const {
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
      cfg.SetString(fqn, value.cast<std::string>());
    } else if (pybind11::isinstance<pybind11::bool_>(value)) {
      cfg.SetBoolean(fqn, value.cast<bool>());
    } else if (pybind11::isinstance<pybind11::int_>(value)) {
      cfg.SetInteger64(fqn, value.cast<int64_t>());
    } else if (pybind11::isinstance<pybind11::float_>(value)) {
      cfg.SetDouble(fqn, value.cast<double>());
    } else if (pybind11::isinstance<pybind11::list>(value) ||
               pybind11::isinstance<pybind11::tuple>(value)) {
      if (cfg.Contains(fqn)) {
        cfg.ClearList(fqn);
      } else {
        cfg.CreateList(fqn);
      }
      ExtractPyIterable(cfg, fqn, value);
    } else if (pybind11::isinstance<pybind11::dict>(value)) {
      cfg.SetGroup(fqn, PyDictToConfiguration(value.cast<pybind11::dict>()));
    } else if (pybind11::isinstance<Config>(value)) {
      cfg.SetGroup(fqn, value.cast<Config>().ImmutableConfig());
    } else {
      if (py_typestr.compare("date") == 0) {
        cfg.SetDate(fqn, PyObjToDate(value));
      } else if (py_typestr.compare("time") == 0) {
        cfg.SetTime(fqn, PyObjToTime(value));
      } else if (py_typestr.compare("datetime") == 0) {
        cfg.SetDateTime(fqn, PyObjToDateTime(value));
      } else {
        throw werkzeugkiste::config::TypeError{err_msg};
      }
    }
  }

  void AppendToList(std::string_view fqn, pybind11::handle value) {
    werkzeugkiste::config::Configuration &cfg = MutableConfig();
    const std::string py_typestr =
        pybind11::cast<std::string>(value.attr("__class__").attr("__name__"));

    if (fqn.empty()) {
      throw werkzeugkiste::config::TypeError{
          "Cannot append a value to the root group!"};
    }

    if (!cfg.Contains(fqn)) {
      // Create list
      cfg.CreateList(fqn);
    } else {
      // Ensure that existing parameter is a list
      if (cfg.Type(fqn) != werkzeugkiste::config::ConfigType::List) {
        std::string msg{"Cannot append to parameter `"};
        msg += fqn;
        msg += "`, because it is a `";
        msg += werkzeugkiste::config::ConfigTypeToString(cfg.Type(fqn));
        msg += "`!";
        throw werkzeugkiste::config::TypeError{msg};
      }
    }

    if (pybind11::isinstance<pybind11::str>(value)) {
      cfg.Append(fqn, value.cast<std::string>());
    } else if (pybind11::isinstance<pybind11::bool_>(value)) {
      cfg.Append(fqn, value.cast<bool>());
    } else if (pybind11::isinstance<pybind11::int_>(value)) {
      cfg.Append(fqn, value.cast<int64_t>());
    } else if (pybind11::isinstance<pybind11::float_>(value)) {
      cfg.Append(fqn, value.cast<double>());
    } else if (pybind11::isinstance<pybind11::list>(value) ||
               pybind11::isinstance<pybind11::tuple>(value)) {
      std::size_t sz = cfg.Size(fqn);
      cfg.AppendList(fqn);
      std::string fqn_nested{fqn};
      fqn_nested += '[';
      fqn_nested += std::to_string(sz);
      fqn_nested += ']';
      ExtractPyIterable(cfg, fqn_nested, value);
    } else if (pybind11::isinstance<pybind11::dict>(value)) {
      cfg.Append(fqn, PyDictToConfiguration(value.cast<pybind11::dict>()));
    } else if (pybind11::isinstance<Config>(value)) {
      cfg.Append(fqn, value.cast<Config>().ImmutableConfig());
    } else {
      if (py_typestr.compare("date") == 0) {
        cfg.Append(fqn, PyObjToDate(value));
      } else if (py_typestr.compare("time") == 0) {
        cfg.Append(fqn, PyObjToTime(value));
      } else if (py_typestr.compare("datetime") == 0) {
        cfg.Append(fqn, PyObjToDateTime(value));
      } else {
        std::string msg{"Cannot append python object of type `"};
        msg += py_typestr;
        msg += "` to parameter list `";
        msg += fqn;
        msg += "`!";
        throw werkzeugkiste::config::TypeError{msg};
      }
    }
  }
};

/// @brief Inserts all elements of the given python list/tuple into the
///   configuration at the given key.
inline void ExtractPyIterable(werkzeugkiste::config::Configuration &cfg,
    std::string_view key,
    pybind11::handle lst) {
  if (!cfg.Contains(key)) {
    std::string msg{"ExtractPyIterable requires that the list parameter `"};
    msg += key;
    msg += "` already exists!";
    throw std::logic_error{msg};
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
      cfg.CreateList(key);
      ExtractPyIterable(cfg, key, item.second);
    } else if (pybind11::isinstance<Config>(item.second)) {
      cfg.SetGroup(key, item.second.cast<Config>().ImmutableConfig());
    } else if (pybind11::isinstance<pybind11::dict>(item.second)) {
      cfg.SetGroup(
          key, PyDictToConfiguration(item.second.cast<pybind11::dict>()));
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
  return pydatetime.attr("time")(
      t.hour, t.minute, t.second, t.nanosecond / 1000);
}

inline pybind11::object DateTimeToPyObj(
    const werkzeugkiste::config::date_time &dt) {
  auto pydatetime = pybind11::module::import("datetime");
  if (dt.IsLocal()) {
    return pydatetime.attr("datetime")(dt.date.year,
        dt.date.month,
        dt.date.day,
        dt.time.hour,
        dt.time.minute,
        dt.time.second,
        dt.time.nanosecond / 1000);
  }

  auto timedelta = pydatetime.attr("timedelta");
  auto timezone = pydatetime.attr("timezone");
  // Ctor parameter order is: (days=0, seconds=0, microseconds=0,
  //   milliseconds=0, minutes=0, hours=0, weeks=0)
  // https://docs.python.org/3/library/datetime.html#datetime.timedelta
  auto offset_td = timedelta(0, 0, 0, 0, dt.offset.value().minutes);
  return pydatetime.attr("datetime")(dt.date.year,
      dt.date.month,
      dt.date.day,
      dt.time.hour,
      dt.time.minute,
      dt.time.second,
      dt.time.nanosecond / 1000,
      timezone(offset_td));
}

inline std::string PyObjToString(pybind11::handle hnd) {
  if (pybind11::isinstance<pybind11::str>(hnd)) {
    return hnd.cast<std::string>();
  } else {
    return pybind11::str(hnd).cast<std::string>();
  }
}
}  // namespace werkzeugkiste::bindings::detail

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H
