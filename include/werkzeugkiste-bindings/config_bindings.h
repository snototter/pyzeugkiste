#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_H

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/logging.h>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace wzkcfg = werkzeugkiste::config;

// TODO nice-to-have: custom C++ exceptions in werkzeugkiste + mapping
// to python exceptions, similar to
// https://pybind11.readthedocs.io/en/stable/advanced/exceptions.html

namespace werkzeugkiste::bindings {

class ConfigWrapper {
 public:
  static ConfigWrapper LoadTOMLFile(std::string_view filename) {
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::Configuration::LoadTOMLFile(filename);
    return instance;
  }

  static ConfigWrapper LoadTOMLString(std::string_view toml_str) {
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::Configuration::LoadTOMLString(toml_str);
    return instance;
  }

  ConfigWrapper() {
    // To initialize an empty config, we simply load an empty TOML string.
    cfg_ = wzkcfg::Configuration::LoadTOMLString("");
  }

  std::string ToTOMLString() const { return cfg_->ToTOML(); }

  std::string ToJSONString() const { return cfg_->ToJSON(); }

  bool Equals(const ConfigWrapper &other) const {
    return cfg_->Equals(other.cfg_.get());
  }

  pybind11::object GetGeneric(std::string_view key) {
    // TODO query type, then call corresponding GetX()
    // TODO how to handle inhomogeneous types, i.e. tables and mixed arrays?
    //   --> initially, support only homogeneous arrays, else raise exception
  }

  void SetBoolean(std::string_view key, bool value) {
    return cfg_->SetBoolean(key, value);
  }

  bool GetBoolean(std::string_view key) const { return cfg_->GetBoolean(key); }

  bool GetBooleanOrDefault(std::string_view key, bool default_value) const {
    return cfg_->GetBooleanOrDefault(key, default_value);
  }

  //  void SetInteger32(std::string_view key, int32_t value) {
  //    return cfg_->SetInteger32(key, value);
  //  }

  //  int32_t GetInteger32(std::string_view key) const {
  //    return cfg_->GetInteger32(key);
  //  }

  //  int32_t GetInteger32OrDefault(std::string_view key, int32_t default_value)
  //  const {
  //    return cfg_->GetInteger32OrDefault(key, default_value);
  //  }

  void SetInteger64(std::string_view key, int64_t value) {
    return cfg_->SetInteger64(key, value);
  }

  int64_t GetInteger64(std::string_view key) const {
    return cfg_->GetInteger64(key);
  }

  int64_t GetInteger64OrDefault(std::string_view key,
                                int64_t default_value) const {
    return cfg_->GetInteger64OrDefault(key, default_value);
  }

  void SetDouble(std::string_view key, double value) {
    return cfg_->SetDouble(key, value);
  }

  double GetDouble(std::string_view key) const { return cfg_->GetDouble(key); }

  double GetDoubleOrDefault(std::string_view key, double default_value) const {
    return cfg_->GetDoubleOrDefault(key, default_value);
  }

  void SetString(std::string_view key, std::string_view value) {
    return cfg_->SetString(key, value);
  }

  std::string GetString(std::string_view key) const {
    return cfg_->GetString(key);
  }

  std::string GetStringOrDefault(std::string_view key,
                                 std::string_view default_value) const {
    return cfg_->GetStringOrDefault(key, default_value);
  }

  // Special functions
  std::vector<std::string> ListParameterNames(
      bool include_array_entries) const {
    return cfg_->ListParameterNames(include_array_entries);
  }

  bool ReplacePlaceholders(
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements) {
    return cfg_->ReplaceStringPlaceholders(replacements);
  }

 private:
  std::unique_ptr<werkzeugkiste::config::Configuration> cfg_{};
};

inline void RegisterScalarAccess(pybind11::class_<ConfigWrapper> &cfg) {
  //---------------------------------------------------------------------------
  // Getting/setting scalars: Boolean
  std::string doc_string = R"doc(
      Changes or creates a :class:`bool` parameter.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::SetBoolean``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.subgroup.my-bool"``.
        value: The value to be set.

      Raises:
        RuntimeError: If ``key`` exists, but is of a different type (changing
          the type is not supported); or if the parent path could not be
          created (*e.g.* if you requested to implicitly create an array).
      )doc";
  cfg.def("set_bool", &ConfigWrapper::SetBoolean, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`bool` parameter or raises an exception.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetBoolean``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.subgroup.my-bool"``.

      Raises:
        RuntimeError: If ``key`` does not exist.
      )doc";
  cfg.def("get_bool", &ConfigWrapper::GetBoolean, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`bool` parameter or the default value.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetBooleanOrDefault``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.my-bool"``.
        default_value: If ``key`` does not exist, this value
          will be returned instead.
      )doc";
  cfg.def("get_bool_or", &ConfigWrapper::GetBooleanOrDefault,
          doc_string.c_str(), pybind11::arg("key"),
          pybind11::arg("default_value"));

  //---------------------------------------------------------------------------
  // Getting/setting scalars: Integer64
  doc_string = R"doc(
      Changes or creates an :class:`int` parameter.

      Note that these bindings assume 64-bit integers, whereas the
      C++ utility differs between 32- and 64-bit representations.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::SetInteger32`` or
      ``werkzeugkiste::config::Configuration::SetInteger64``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.subgroup.my-int"``.
        value: The value to be set.

      Raises:
        RuntimeError: If ``key`` exists, but is of a different type (changing
          the type is not supported); or if the parent path could not be
          created (*e.g.* if you requested to implicitly create an array).
      )doc";
  cfg.def("set_int", &ConfigWrapper::SetInteger64, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`int` parameter or raises an exception.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetInteger32`` or
      ``werkzeugkiste::config::Configuration::GetInteger64``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.subgroup.my-int"``.

      Raises:
        RuntimeError: If ``key`` does not exist.
      )doc";
  cfg.def("get_int", &ConfigWrapper::GetInteger64, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`int` parameter or the default value.

      Note that these bindings assume 64-bit integers, whereas the
      C++ utility differs between 32- and 64-bit representations.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetInteger32OrDefault`` or
      ``werkzeugkiste::config::Configuration::GetInteger64OrDefault``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.my-integer"``.
        default_value: If the parameter does not exist, this value
          will be returned instead.
      )doc";
  cfg.def("get_int_or", &ConfigWrapper::GetInteger64OrDefault,
          doc_string.c_str(), pybind11::arg("key"),
          pybind11::arg("default_value"));

  //---------------------------------------------------------------------------
  // Getting/setting scalars: Double
  doc_string = R"doc(
      Changes or creates a :class:`float` parameter.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::SetDouble``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.subgroup.my-flt"``.
        value: The value to be set.

      Raises:
        RuntimeError: If ``key`` exists, but is of a different type (changing
          the type is not supported); or if the parent path could not be
          created (*e.g.* if you requested to implicitly create an array).
      )doc";
  cfg.def("set_double", &ConfigWrapper::SetDouble, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`float` parameter or raises an exception.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetDouble``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.subgroup.my-flt"``.

      Raises:
        RuntimeError: If ``key`` does not exist.
      )doc";
  cfg.def("get_double", &ConfigWrapper::GetDouble, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`float` parameter or the default value.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetDoubleOrDefault``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.my-floating-point-number"``.
        default_value: If the parameter does not exist, this value
          will be returned instead.
      )doc";
  cfg.def("get_double_or", &ConfigWrapper::GetDoubleOrDefault,
          doc_string.c_str(), pybind11::arg("key"),
          pybind11::arg("default_value"));

  //---------------------------------------------------------------------------
  // Getting/setting scalars: String
  doc_string = R"doc(
      Changes or creates a :class:`str` parameter.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::SetString``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.subgroup.my-str"``.
        value: The value to be set.

      Raises:
        RuntimeError: If ``key`` exists, but is of a different type (changing
          the type is not supported); or if the parent path could not be
          created (*e.g.* if you requested to implicitly create an array).
      )doc";
  cfg.def("set_str", &ConfigWrapper::SetString, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`str` parameter or raises an exception.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetString``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.subgroup.my-str"``.

      Raises:
        RuntimeError: If ``key`` does not exist.
      )doc";
  cfg.def("get_str", &ConfigWrapper::GetString, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`str` parameter or the default value.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetStringOrDefault``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.my-str"``.
        default_value: If the parameter does not exist, this value
          will be returned instead.
      )doc";
  cfg.def("get_str_or", &ConfigWrapper::GetStringOrDefault, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));
}

inline void RegisterConfiguration(pybind11::module &m) {
  const std::string module_name = m.attr("__name__").cast<std::string>();
  const std::string config_name = std::string{module_name} + ".Configuration";

  std::string doc_string{};
  std::ostringstream doc_stream;
  doc_stream
      << "TODO A :class:`~" << config_name << "`.\n\n"
      << "**Corresponding C++ API:** ``werkzeugkiste::config::Configuration``.";

  pybind11::class_<ConfigWrapper> cfg(m, "Configuration",
                                      doc_stream.str().c_str());

  cfg.def(pybind11::init<>(), "Creates an empty configuration.");

  //---------------------------------------------------------------------------
  // Loading a configuration
  std::ostringstream().swap(doc_stream);
  doc_stream << "Returns a :class:`~" << config_name
             << "` loaded from the given TOML file.";

  cfg.def_static("load_toml_file", &ConfigWrapper::LoadTOMLFile,
                 doc_stream.str().c_str(), pybind11::arg("filename"));

  std::ostringstream().swap(doc_stream);
  doc_stream << "Returns a :class:`~" << config_name
             << "` loaded from the given TOML string.";
  cfg.def_static("load_toml_string", &ConfigWrapper::LoadTOMLString,
                 doc_stream.str().c_str(), pybind11::arg("toml_str"));

  //---------------------------------------------------------------------------
  // Serializing
  cfg.def("to_toml_str", &ConfigWrapper::ToTOMLString,
          "Returns a formatted `TOML <https://toml.io/>`__ representation "
          "of this configuration.");

  cfg.def("to_json_str", &ConfigWrapper::ToJSONString,
          "Returns a formatted `JSON <https://www.json.org/>`__ representation "
          "of this configuration.");

  RegisterScalarAccess(cfg);

  //---------------------------------------------------------------------------
  // Getting/Setting lists/tuples/pairs
  // TODO

  //---------------------------------------------------------------------------
  // Special utils
  // TODO load nested, replace placeholders, adjust paths

  doc_string = R"doc(
      Returns the fully-qualified names/keys of all parameters.

      The key defines the "path" from the configuration's root node
      to the parameter.

      Args:
        include_array_entries: If ``True``, the name of each parameter will
          be returned, *i.e.* the result will also include **each** array
          element. Otherwise, only explicitly named parameters will be
          included, refer to the example below.

      .. code-block:: toml
         :caption: Exemplary configuration

         str1 = 'value'

         [values.numeric]
         int1 = 42
         flt1 = 1e-3
         arr1 = [1, 2, 3]

         [values.other]
         str2 = 'value'
         time1 = 08:00:00
         arr2 = [
           'value',
           { int2 = 123, flt2 = 1.5 }
         ]

      .. code-block:: python
         :caption: Extracted parameter names

         named_parameters = [  # list_parameter_names(False)
           'str1',
           'values',
           'values.numeric',
           'values.numeric.arr1',
           'values.numeric.flt1',
           'values.numeric.int1',
           'values.other',
           'values.other.arr2',
           'values.other.arr2[1].flt2',
           'values.other.arr2[1].int2',
           'values.other.str2',
           'values.other.time1'
         ]

         named_parameters = [  # list_parameter_names(True)
           'str1',
           'values',
           'values.numeric',
           'values.numeric.arr1',
           'values.numeric.arr1[0]',
           'values.numeric.arr1[1]',
           'values.numeric.arr1[2]',
           'values.numeric.flt1',
           'values.numeric.int1',
           'values.other',
           'values.other.arr2',
           'values.other.arr2[0]',
           'values.other.arr2[1]',
           'values.other.arr2[1].flt2',
           'values.other.arr2[1].int2',
           'values.other.str2',
           'values.other.time1'
         ]

      )doc";
  cfg.def("list_parameter_names", &ConfigWrapper::ListParameterNames,
          doc_string.c_str(), pybind11::arg("include_array_entries") = false);

  doc_string = R"doc(
      Replaces **all occurrences** of the given string placeholders.

      Applies string replacement to all :class:`str` parameters of this
      configuration.

      Note:
        The string replacements will be applied in the order specified
        by the ``replacements`` parameter. To avoid any unwanted side effects,
        choose **unique placeholders** that are not contained in any other
        string parameter value or a replacement value.

      Args:
        replacements: A :class:`list` of ``(search_str, replacement_str)``
          pairs, *i.e.* a :class:`tuple` of :class:`str`.

      Return:
        ``True`` if any placeholder has actually been replaced.

      .. code-block:: toml
         :caption: Exemplary configuration

         str = 'Release: %VERSION%'
         token = '%TOKEN% - %TOKEN%'

      .. code-block:: python
         :caption: Replace placeholders

         cfg.replace_placeholders([
             ('%VERSION%', 'v0.1'), ('%TOKEN%', '1337')])

      .. code-block:: toml
         :caption: Resulting configuration

         str = 'Release: v0.1'
         token = '1337 - 1337'
      )doc";
  cfg.def("replace_placeholders", &ConfigWrapper::ReplacePlaceholders,
          doc_string.c_str(), pybind11::arg("placeholders"));

  // Equality checks
  cfg.def(
      "__eq__",
      [](const ConfigWrapper &a, const ConfigWrapper &b) -> bool {
        return a.Equals(b);
      },
      "Checks for equality.\n\nReturns ``True`` if both configs contain the "
      "exact same configuration, *i.e.* keys, corresponding data types and\n"
      "values.",
      pybind11::arg("other"));

  std::ostringstream().swap(doc_stream);
  doc_stream << "Checks for inequality, see :meth:`~." << config_name
             << ".__eq__` for details.";
  cfg.def(
      "__ne__",
      [](const ConfigWrapper &a, const ConfigWrapper &b) -> bool {
        return !a.Equals(b);
      },
      doc_stream.str().c_str(), pybind11::arg("other"));

  // TODO __str__ and __repr__, e.g. (x 1st level keys, y parameters in total)
}
}  // namespace werkzeugkiste::bindings

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_H
