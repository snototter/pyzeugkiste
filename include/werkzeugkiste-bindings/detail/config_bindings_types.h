#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H

#include <pybind11/chrono.h>
#include <pybind11/numpy.h>
#include <pybind11/eigen.h>
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
    const std::string fqn_src_elem =
        werkzeugkiste::config::Configuration::KeyForListElement(
          fqn_src, idx);
    
    switch (src.Type(fqn_src_elem)) {
      case werkzeugkiste::config::ConfigType::Boolean:
        dst.Append(fqn_dst, src.GetBoolean(fqn_src_elem));
        break;

      case werkzeugkiste::config::ConfigType::Integer:
        dst.Append(fqn_dst, src.GetInt64(fqn_src_elem));
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
        const std::size_t size_dst = dst.Size(fqn_dst);
        const std::string fqn_dst_elem =
            werkzeugkiste::config::Configuration::KeyForListElement(
              fqn_dst, size_dst);
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

/// @brief Returns a copy of the matrix as pybind11::array.
///
/// Currently, `MatToArray` will cause an unnecessary copy since the matrix
/// has already been allocated by werkzeugkiste to convert a nested list into
/// a matrix.
/// However, since querying configuration parameters is typically not used in
/// performance-critical code sections, this overhead seems acceptable for now.
///
/// @tparam Tp Type of the matrix' coefficients.
/// @param mat The row-major(!) matrix.
template <typename Tp>
pybind11::array_t<Tp> MatToArray(const werkzeugkiste::config::Matrix<Tp> &mat){
  static_assert(mat.IsRowMajor,
      "Matrix retrieved from configuration must be in row-major order!");
  // By design, we always return a 2-dim matrix:
  const std::vector<ssize_t> shape{mat.rows(), mat.cols()};
  return pybind11::array_t<Tp>(
      shape,  // Shape (height, width)
      {shape[1] * sizeof(Tp), sizeof(Tp)},  // Strides (row stride, col stride)
      mat.data());
}

/// @brief Holds the actual configuration data (to enable shared memory usage
///   among the Config instances).
struct DataHolder {
  werkzeugkiste::config::Configuration data{};
};

class Config {
 public:
  //---------------------------------------------------------------------------
  // Iterator
  struct Iterator {
    Iterator(Config const *cfg,
        werkzeugkiste::config::ConfigType type,
        std::size_t idx)
        : cfg_{cfg},
          is_group_{type == werkzeugkiste::config::ConfigType::Group},
          idx_{idx} {
      if (is_group_) {
        keys_ = cfg->Keys();
      }
    }

    pybind11::object operator*() {
      if (is_group_) {
        return pybind11::str(keys_[idx_]);
      }
      return cfg_->GetValue(idx_);
    }

    // Prefix increment
    Iterator &operator++() {
      ++idx_;
      return *this;
    }

    // Postfix increment
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    friend bool operator==(const Iterator &a, const Iterator &b) {
      if (a.idx_ != b.idx_) {
        return false;
      }
      return a.cfg_->Equals(*b.cfg_);
    }

    friend bool operator!=(const Iterator &a, const Iterator &b) {
      return !(a == b);
    }

   private:
    Config const *cfg_{nullptr};
    bool is_group_{false};
    std::size_t idx_{0};
    std::vector<std::string> keys_{};
  };

  Iterator cbegin() const { return Iterator(this, Type(), 0); }
  Iterator cend() const { return Iterator(this, Type(), Length()); }

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

  static Config LoadJSONFile(pybind11::handle filename,
      werkzeugkiste::config::NullValuePolicy none_policy) {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data = werkzeugkiste::config::LoadJSONFile(
        PyObjToString(filename), none_policy);
    return cfg;
  }

  static Config LoadJSONString(std::string_view json_str,
      werkzeugkiste::config::NullValuePolicy none_policy) {
    Config cfg{};
    cfg.data_ = std::make_shared<DataHolder>();
    cfg.data_->data =
        werkzeugkiste::config::LoadJSONString(json_str, none_policy);
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

  bool Equals(pybind11::handle other) const {
    if (pybind11::isinstance<Config>(other)) {
      return Equals(other.cast<Config>());
    }

    if (pybind11::isinstance<pybind11::dict>(other)) {
      return CopyViewedGroup().Equals(
          PyDictToConfiguration(other.cast<pybind11::dict>()));
    }

    std::string msg{"Cannot compare a `Config` instance against a `"};
    msg +=
        pybind11::cast<std::string>(other.attr("__class__").attr("__name__"));
    msg += "`!";
    throw werkzeugkiste::config::TypeError{msg};
  }

  bool Contains(std::string_view key) const {
    // If implemented for a sequence type (i.e. list), __contains__ should
    // check for equality of the values:
    // https://docs.python.org/3/reference/datamodel.html?emulating-container-types#emulating-container-types
    // Thus, we only support __contains__ for groups:
    if (Type() == werkzeugkiste::config::ConfigType::List) {
      throw werkzeugkiste::config::TypeError{
          "`__contains__` is not supported for a Config view of a list!"};
    }
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
  pybind11::object GetScalarOrView(std::string_view key) {
    return GetBuiltinOrView(Key(key));
  }

  /// @brief Enables `__getitem__[int]` for parameters of type `list`.
  pybind11::object GetScalarOrView(int index) {
    return GetBuiltinOrView(Key(index));
  }

  /// @brief Returns a copy of the parameter as plain python type, *i.e.*
  ///   parameter lists/groups will be converted to lists and dictionaries.
  pybind11::object GetValue(int index) const {
    return GetBuiltinValue(Key(index));
  }

  /// @brief Returns a copy of the parameter as plain python type, *i.e.*
  ///   parameter lists/groups will be converted to lists and dictionaries.
  pybind11::object GetValue(std::string_view key) const {
    return GetBuiltinValue(Key(key));
  }

  pybind11::list Values() const {
    // Type check (group vs list) is implicitly handled by Keys(),
    // which is only supported for groups.
    pybind11::list lst;
    for (const auto &key : Keys()) {
      lst.append(GetValue(key));
    }
    return lst;
  }

  pybind11::list Items() const {
    // Type check (group vs list) is implicitly handled by Keys(),
    // which is only supported for groups.
    pybind11::list lst;
    for (const auto &key : Keys()) {
      lst.append(pybind11::make_tuple(key, GetValue(key)));
    }
    return lst;
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
  
  pybind11::array GetMatrix(std::string_view key,
      const pybind11::type &dtype) const {
    // Cannot use builtins like pybind11::type::of<double>, see
    // https://github.com/pybind/pybind11/issues/2486
    // Instead, we can us the following string comparison to determine the
    // requested output dtype:
    const std::string tp_name = pybind11::cast<std::string>(
        dtype.attr("__name__"));
    const std::string fqn = Key(key);

    if (tp_name.compare("float64") == 0) {
      return MatToArray(ImmutableConfig().GetMatrixDouble(fqn));
    }
    
    if (tp_name.compare("int64") == 0) {
      return MatToArray(ImmutableConfig().GetMatrixInt64(fqn));
    }

    if (tp_name.compare("int32") == 0) {
      return MatToArray(ImmutableConfig().GetMatrixInt32(fqn));
    }

    if (tp_name.compare("uint8") == 0) {
      return MatToArray(ImmutableConfig().GetMatrixUInt8(fqn));
    }

    std::string msg{"Converting the configuration parameter `"};
    msg += fqn;
    msg += "` to a NumPy array of dtype=`" + tp_name
        + "` is not yet supported! Please file a feature request at: "
          "https://github.com/snototter/pyzeugkiste/issues";
    throw werkzeugkiste::config::TypeError{msg};
  }

  pybind11::object GetMatrixOr(std::string_view key,
      const pybind11::dtype &dtype,
      const pybind11::object &def) const {
    if (!Contains(key)) {
      return def;
    }
    return GetMatrix(key, dtype);
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

  werkzeugkiste::config::Configuration CopyGroup(std::string_view fqn) const {
    const werkzeugkiste::config::Configuration &cfg = ImmutableConfig();
    if (fqn.empty()) {
      return cfg;
    }
    if (cfg.Type(fqn) == werkzeugkiste::config::ConfigType::List) {
      using namespace std::string_view_literals;
      werkzeugkiste::config::Configuration copy{};
      copy.CreateList("list"sv);
      CopyList(cfg, fqn, copy, "list"sv);
      return copy;
    }
    return cfg.GetGroup(fqn);
  }

  inline werkzeugkiste::config::Configuration CopyViewedGroup() const {
    return CopyGroup(fqn_prefix_);
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
      return werkzeugkiste::config::Configuration::KeyForListElement(
        fqn_prefix_, index);
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
      const std::string elem_key = 
          werkzeugkiste::config::Configuration::KeyForListElement(
            fqn, idx);
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
        return pybind11::int_{cfg.GetInt64(fqn)};

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

  pybind11::object GetBuiltinOrView(std::string_view fqn) {
    const werkzeugkiste::config::Configuration &cfg = ImmutableConfig();
    const werkzeugkiste::config::ConfigType type = cfg.Type(fqn);

    if ((type == werkzeugkiste::config::ConfigType::List) ||
        (type == werkzeugkiste::config::ConfigType::Group)) {
      pybind11::object config_cls =
          pybind11::module::import("pyzeugkiste._core._cfg").attr("Config");
      pybind11::object obj = config_cls();
      auto *ptr = obj.cast<Config *>();
      ptr->data_ = data_;
      ptr->fqn_prefix_ = fqn;
      return obj;
    }

    return ValueOr(type, fqn, false);
  }

  pybind11::object GetBuiltinValue(std::string_view fqn) const {
    const werkzeugkiste::config::Configuration &cfg = ImmutableConfig();
    const werkzeugkiste::config::ConfigType type = cfg.Type(fqn);
    return ValueOr(type, fqn, false);
  }

  template <typename TpNumpy>
  void SetMatrixHelper(std::string_view fqn, pybind11::array arr) {
    static_assert(std::is_arithmetic_v<TpNumpy>,
        "Only numpy arrays of arithmetic types are supported!");
    using TpCfg =
        std::conditional_t<std::is_integral_v<TpNumpy>, int64_t, double>;

    werkzeugkiste::config::Configuration &cfg = MutableConfig();
    if (cfg.EnsureTypeIfExists(fqn, werkzeugkiste::config::ConfigType::List)) {
      cfg.ClearList(fqn);
    } else {
      cfg.CreateList(fqn);
    }

    const std::size_t num_rows = static_cast<std::size_t>(arr.shape(0));
    if (arr.ndim() == 1) {
      auto proxy = arr.unchecked<TpNumpy, 1>();
      for (std::size_t row = 0; row < num_rows; ++row) {
        cfg.Append(fqn, werkzeugkiste::config::checked_numcast<TpCfg, TpNumpy, werkzeugkiste::config::TypeError>(proxy(row)));
      }
    } else {
      auto proxy = arr.unchecked<TpNumpy, 2>();
      const std::size_t num_cols = static_cast<std::size_t>(arr.shape(1));
      for (std::size_t row = 0; row < num_rows; ++row) {
        const std::string nested_fqn = cfg.KeyForListElement(fqn, row);
        cfg.AppendList(fqn);
        for (std::size_t col = 0; col < num_cols; ++col) {
          cfg.Append(nested_fqn, werkzeugkiste::config::checked_numcast<TpCfg, TpNumpy, werkzeugkiste::config::TypeError>(proxy(row, col)));
        }
      }
    }
  }

  void SetMatrix(std::string_view fqn, const pybind11::array &arr) {
    if (pybind11::isinstance<pybind11::array_t<uint8_t>>(arr)) {
      SetMatrixHelper<uint8_t>(fqn, arr);
    } else if (pybind11::isinstance<pybind11::array_t<int16_t>>(arr)) {
      SetMatrixHelper<int16_t>(fqn, arr);
    } else if (pybind11::isinstance<pybind11::array_t<uint16_t>>(arr)) {
      SetMatrixHelper<uint16_t>(fqn, arr);
    } else if (pybind11::isinstance<pybind11::array_t<int32_t>>(arr)) {
      SetMatrixHelper<int32_t>(fqn, arr);
    } else if (pybind11::isinstance<pybind11::array_t<uint32_t>>(arr)) {
      SetMatrixHelper<uint32_t>(fqn, arr);
    } else if (pybind11::isinstance<pybind11::array_t<int64_t>>(arr)) {
      SetMatrixHelper<int64_t>(fqn, arr);
    } else if (pybind11::isinstance<pybind11::array_t<uint64_t>>(arr)) {
      SetMatrixHelper<uint64_t>(fqn, arr);
    } else if (pybind11::isinstance<pybind11::array_t<float>>(arr)) {
      SetMatrixHelper<float>(fqn, arr);
    } else if (pybind11::isinstance<pybind11::array_t<double>>(arr)) {
      SetMatrixHelper<double>(fqn, arr);
    } else {
      const pybind11::dtype dtype = arr.dtype();
      std::string msg{"Cannot set parameter `"};
      msg += fqn;
      msg += "` from numpy array with incompatible `dtype ("
          + pybind11::cast<std::string>(dtype.attr("name")) + ", \"";
      const pybind11::list dt_descr = pybind11::cast<pybind11::list>(dtype.attr("descr"));
      for (std::size_t i = 0; i < dt_descr.size(); ++i) {
        // First element holds the optional name, second one holds the
        // type description we're interested in, check for example:
        // https://numpy.org/doc/stable/reference/generated/numpy.dtype.descr.html
        const pybind11::tuple td = pybind11::cast<pybind11::tuple>(dt_descr[i]);
        msg += pybind11::cast<std::string>(td[1]);
        if (i < dt_descr.size() - 1) {
          msg += "\", \"";
        }
      }
      msg += ")`!";
      throw werkzeugkiste::config::TypeError{msg};
    }
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
      cfg.SetInt64(fqn, value.cast<int64_t>());
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
    } else if (pybind11::isinstance<pybind11::array>(value)) {
      SetMatrix(fqn, value.cast<pybind11::array>());
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
      const std::size_t size_list = cfg.Size(fqn);
      const std::string fqn_nested =
          werkzeugkiste::config::Configuration::KeyForListElement(
            fqn, size_list);
      cfg.AppendList(fqn);
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
      const std::size_t size_list = cfg.Size(key);
      const std::string elem_key =
          werkzeugkiste::config::Configuration::KeyForListElement(
            key, size_list);
      cfg.AppendList(key);
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
      cfg.SetInt64(key, item.second.cast<int64_t>());
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
