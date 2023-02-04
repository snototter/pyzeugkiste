#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_H

#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/logging.h>

#include <sstream>
#include <stdexcept>
#include <string_view>
#include <memory>

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

  bool GetBoolean(const std::string &key) {//std::string_view key) {
    return cfg_->GetBoolean(key);
  }

//  bool GetBooleanOrDefault(std::string_view key, bool default_value) {
//    return cfg_->GetBooleanOrDefault(key, default_value);
//  }

//  int32_t GetInteger32(std::string_view key) {
//    return cfg_->GetInteger32(key);
//  }


private:
  std::unique_ptr<werkzeugkiste::config::Configuration> cfg_{};
};

inline void RegisterConfiguration(pybind11::module &m) {
  const std::string module_name = m.attr("__name__").cast<std::string>();

  std::ostringstream doc;
  doc << "TODO A :class:`~"
      << module_name << ".Configuration`.\n\n"
      << "**Corresponding C++ API:** ``" << module_name << "::Configuration``.";
  pybind11::class_<ConfigWrapper> cfg(m, "Configuration", doc.str().c_str());

  // Loading a configuration
  cfg.def_static("load_toml_file", &ConfigWrapper::LoadTOMLFile,
                 "TODO", pybind11::arg("filename"));
  cfg.def_static("load_toml_string", &ConfigWrapper::LoadTOMLString,
                 "TODO", pybind11::arg("toml_str"));

  // Serializing
  cfg.def("to_toml", &ConfigWrapper::ToTOMLString,
          "TODO");

  cfg.def("to_jsong", &ConfigWrapper::ToJSONString,
          "TODO");
  // Getting/Setting scalars
  // TODO
  cfg.def("get_bool", &ConfigWrapper::GetBoolean,
          "TODO", pybind11::arg("key"));
//  cfg.def("get_bool_or", &ConfigWrapper::GetBooleanOrDefault,
//          "TODO", pybind11::arg("key"), pybind11::arg("default"));

//  cfg.def("get_int32", &ConfigWrapper::GetInteger32,
//          "TODO", pybind11::arg("key"));

  // Getting/Setting lists/tuples/pairs
  // TODO

  // Special utils
  // TODO load nested, replace placeholders
}
}  // namespace werkzeugkiste::bindings

#endif // WERKZEUGKISTE_BINDINGS_CONFIG_H

