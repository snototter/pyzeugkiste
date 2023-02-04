#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_H

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
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

  std::string ToTOMLString() const {
    return cfg_->ToTOML();
  }

  std::string ToJSONString() const {
    return cfg_->ToJSON();
  }

  bool Equals(const ConfigWrapper &other) const {
    return cfg_->Equals(other.cfg_.get());
  }

  void SetBoolean(std::string_view key, bool value) {
    return cfg_->SetBoolean(key, value);
  }

  bool GetBoolean(std::string_view key) const {
    return cfg_->GetBoolean(key);
  }

  bool GetBooleanOrDefault(std::string_view key, bool default_value) const {
    return cfg_->GetBooleanOrDefault(key, default_value);
  }

  void SetInteger32(std::string_view key, int32_t value) {
    return cfg_->SetInteger32(key, value);
  }

  int32_t GetInteger32(std::string_view key) const {
    return cfg_->GetInteger32(key);
  }

  int32_t GetInteger32OrDefault(std::string_view key, int32_t default_value) const {
    return cfg_->GetInteger32OrDefault(key, default_value);
  }

  void SetInteger64(std::string_view key, int64_t value) {
    return cfg_->SetInteger64(key, value);
  }

  int64_t GetInteger64(std::string_view key) const {
    return cfg_->GetInteger64(key);
  }

  int64_t GetInteger64OrDefault(std::string_view key, int64_t default_value) const {
    return cfg_->GetInteger64OrDefault(key, default_value);
  }

  void SetDouble(std::string_view key, double value) {
    return cfg_->SetDouble(key, value);
  }

  double GetDouble(std::string_view key) const {
    return cfg_->GetDouble(key);
  }

  double GetDoubleOrDefault(std::string_view key, double default_value) const {
    return cfg_->GetDoubleOrDefault(key, default_value);
  }

  void SetString(std::string_view key, std::string_view value) {
    return cfg_->SetString(key, value);
  }

  std::string GetString(std::string_view key) const {
    return cfg_->GetString(key);
  }

  std::string GetStringOrDefault(std::string_view key, std::string_view default_value) const {
    return cfg_->GetStringOrDefault(key, default_value);
  }


  // Special functions
  std::vector<std::string> ParameterNames() const {
    return cfg_->ParameterNames();
  }

  bool ReplacePlaceholders(const std::vector<std::pair<std::string_view, std::string_view>> &replacements) {
    return cfg_->ReplaceStringPlaceholders(replacements);
  }

private:
  std::unique_ptr<werkzeugkiste::config::Configuration> cfg_{};
};

inline void RegisterConfiguration(pybind11::module &m) {
  const std::string module_name = m.attr("__name__").cast<std::string>();
  const std::string config_name = module_name + ".Configuration";

  std::string doc_string{};
  std::ostringstream doc_stream;
  doc_stream << "TODO A :class:`~" << config_name  << "`.\n\n"
      << "**Corresponding C++ API:** ``"
      << module_name << "::Configuration``.";
  pybind11::class_<ConfigWrapper> cfg(m, "Configuration", doc_stream.str().c_str());

  // Loading a configuration
  std::ostringstream().swap(doc_stream);
  doc_stream << "Returns a :class:`~" << module_name
      << ".Configuration loaded from the given TOML file.";
  cfg.def_static("load_toml_file", &ConfigWrapper::LoadTOMLFile,
                 doc_stream.str().c_str(), pybind11::arg("filename"));

  std::ostringstream().swap(doc_stream);
  doc_stream << "Returns a :class:`~" << module_name
      << ".Configuration loaded from the given TOML string.";
  cfg.def_static("load_toml_string", &ConfigWrapper::LoadTOMLString,
                 doc_stream.str().c_str(), pybind11::arg("toml_str"));

  // Serializing
  cfg.def("to_toml_str", &ConfigWrapper::ToTOMLString,
          "Returns a formatted TOML representation of this configuration.");

  cfg.def("to_json_str", &ConfigWrapper::ToJSONString,
          "Returns a formatted JSON representation of this configuration.");


  // Getting/setting scalars
  // TODO
  cfg.def("set_bool", &ConfigWrapper::SetBoolean,
          "TODO", pybind11::arg("key"), pybind11::arg("value"));
  cfg.def("get_bool", &ConfigWrapper::GetBoolean,
          "TODO", pybind11::arg("key"));
  cfg.def("get_bool_or", &ConfigWrapper::GetBooleanOrDefault,
          "TODO", pybind11::arg("key"), pybind11::arg("default_value"));


  cfg.def("set_int32", &ConfigWrapper::SetInteger32,
          "TODO", pybind11::arg("key"), pybind11::arg("value"));
  cfg.def("get_int32", &ConfigWrapper::GetInteger32,
          "TODO", pybind11::arg("key"));
  cfg.def("get_int32_or", &ConfigWrapper::GetInteger32OrDefault,
          "TODO", pybind11::arg("key"), pybind11::arg("default_value"));


  cfg.def("set_int64", &ConfigWrapper::SetInteger64,
          "TODO", pybind11::arg("key"), pybind11::arg("value"));
  cfg.def("get_int64", &ConfigWrapper::GetInteger64,
          "TODO", pybind11::arg("key"));
  cfg.def("get_int64_or", &ConfigWrapper::GetInteger64OrDefault,
          "TODO", pybind11::arg("key"), pybind11::arg("default_value"));


  cfg.def("set_double", &ConfigWrapper::SetDouble,
          "TODO", pybind11::arg("key"), pybind11::arg("value"));
  cfg.def("get_double", &ConfigWrapper::GetDouble,
          "TODO double-precision", pybind11::arg("key"));
  cfg.def("get_double_or", &ConfigWrapper::GetDoubleOrDefault,
          "Returns the :class:`float` parameter if the given key exists, or the default value otherwise.", pybind11::arg("key"), pybind11::arg("default_value"));

  cfg.def("set_str", &ConfigWrapper::SetString,
          "TODO", pybind11::arg("key"), pybind11::arg("value"));
  cfg.def("get_str", &ConfigWrapper::GetString,
          "TODO", pybind11::arg("key"));
  cfg.def("get_str_or", &ConfigWrapper::GetStringOrDefault,
          "Returns the :class:`str` parameter if the given key exists, or the default value otherwise.",
          pybind11::arg("key"), pybind11::arg("default_value"));

  // Getting/Setting lists/tuples/pairs
  // TODO

  // Special utils
  // TODO load nested, replace placeholders, adjust paths
  cfg.def("list_parameters", &ConfigWrapper::ParameterNames,
          "Returns a list of all parameter names (*i.e.*, their fully-qualified keys).");

  doc_string = R"doc(
    Visits all string parameters and replaces *all* occurrences of the
    given needle/replacement pairs.

    Note that the string replacements will be applied in the order specified
    by the `replacements` parameter. To avoid any unwanted side effects,
    choose unique placeholders that are not contained in any other string
    parameter value (or a replacement value).

    Args:
      replacements: A :class:`list` of `(search_str, replacement_str)` pairs.

    Return:
      `True` if any placeholder has actually been replaced.

    Example:
      >>> TODO
    )doc";
  cfg.def("replace_placeholders", &ConfigWrapper::ReplacePlaceholders,
          doc_string.c_str(), pybind11::arg("placeholders"));


  // Equality checks
  cfg.def(
      "__eq__", [](const ConfigWrapper &a, const ConfigWrapper &b) -> bool {
          return a.Equals(b);
      },
      "Returns `True` if both configs contain the exact same parameters (keys, types and values).",
      pybind11::arg("other"));

  std::ostringstream().swap(doc_stream);
  doc_stream << "Checks for inequality, see :meth:`~." << module_name
      << ".Configuration.__eq__` for details.";
  cfg.def(
      "__ne__", [](const ConfigWrapper &a, const ConfigWrapper &b) -> bool {
          return !a.Equals(b);
      },
      doc_stream.str().c_str(), pybind11::arg("other"));

  //TODO __str__ and __repr__, e.g. (x 1st level keys, y parameters in total)
}
}  // namespace werkzeugkiste::bindings

#endif // WERKZEUGKISTE_BINDINGS_CONFIG_H

