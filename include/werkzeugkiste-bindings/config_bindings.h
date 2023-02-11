#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_H

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <werkzeugkiste/config/casts.h>
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

/*
from pyzeugkiste import config as cfg

c = cfg.Configuration.load_toml_string("""
    my-bool = true
    int = 1
    flt = 3.5
    str = 'value'
    day = 2022-12-01
    lst = [1, 2, 3, 4]
    mixed1 = [1, 3.5, 'str']
    mixed2 = [1, 'value', { key = 'foo', value = 'bar' }]

    [tbl]
    vi = 2
    vf = 1.5e3
    """)
c.list_parameter_names()

c['str']
c['flt']
c['float']
c['int']
c['day']
c['lst']
c['mixed1']
c['mixed2']
c['tbl']
c['tbl.vi']

c['my-bool'] = False # Should work
c['my-bool']
c['my-bool'] = 'fail' # Should fail

c['tbl.vi'] = 42
c['tbl.vi'] = 0.5
c['tbl.vi'] = True
c['tbl.vi'] = 'abc'
c['tbl.vi'] = [1, 2]
c['tbl.vi']

c['str'] = 3

'str' in c
'tbl' in c

c['foo'] = 123.2
c['foo']
c['foo'] = 10
c['foo']
c['foo'] = False # fail

*/

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

  std::string ToTOMLString() const { return cfg_.ToTOML(); }

  std::string ToJSONString() const { return cfg_.ToJSON(); }

  bool Equals(const ConfigWrapper &other) const {
    return cfg_.Equals(other.cfg_);
  }

  bool Empty() const { return cfg_.Empty(); }

  bool Contains(std::string_view key) const { return cfg_.Contains(key); }

  wzkcfg::ConfigType Type(std::string_view key) const { return cfg_.Type(key); }

  wzkcfg::Configuration GetGroup(std::string_view key) const {
    return cfg_.GetGroup(key);
  }

  void ReplaceConfig(wzkcfg::Configuration &&c) { cfg_ = std::move(c); }

  //---------------------------------------------------------------------------
  // Scalar data types

  void SetBoolean(std::string_view key, bool value) {
    cfg_.SetBoolean(key, value);
  }

  bool GetBoolean(std::string_view key) const { return cfg_.GetBoolean(key); }

  bool GetBooleanOr(std::string_view key, bool default_value) const {
    return cfg_.GetBooleanOr(key, default_value);
  }

  void SetInteger64(std::string_view key, int64_t value) {
    cfg_.SetInteger64(key, value);
  }

  int64_t GetInteger64(std::string_view key) const {
    return cfg_.GetInteger64(key);
  }

  int64_t GetInteger64Or(std::string_view key, int64_t default_value) const {
    return cfg_.GetInteger64Or(key, default_value);
  }

  void SetDouble(std::string_view key, double value) {
    cfg_.SetDouble(key, value);
  }

  double GetDouble(std::string_view key) const { return cfg_.GetDouble(key); }

  double GetDoubleOr(std::string_view key, double default_value) const {
    return cfg_.GetDoubleOr(key, default_value);
  }

  void SetString(std::string_view key, std::string_view value) {
    cfg_.SetString(key, value);
  }

  std::string GetString(std::string_view key) const {
    return cfg_.GetString(key);
  }

  std::string GetStringOr(std::string_view key,
                          std::string_view default_value) const {
    return cfg_.GetStringOr(key, default_value);
  }

  // Special functions
  std::vector<std::string> ListParameterNames(
      bool include_array_entries) const {
    return cfg_.ListParameterNames(include_array_entries);
  }

  bool ReplacePlaceholders(
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements) {
    return cfg_.ReplaceStringPlaceholders(replacements);
  }

 private:
  wzkcfg::Configuration cfg_{};
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
      ``werkzeugkiste::config::Configuration::GetBooleanOr``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.my-bool"``.
        default_value: If ``key`` does not exist, this value
          will be returned instead.
      )doc";
  cfg.def("get_bool_or", &ConfigWrapper::GetBooleanOr, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));

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
      ``werkzeugkiste::config::Configuration::GetInteger32Or`` or
      ``werkzeugkiste::config::Configuration::GetInteger64Or``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.my-integer"``.
        default_value: If the parameter does not exist, this value
          will be returned instead.
      )doc";
  cfg.def("get_int_or", &ConfigWrapper::GetInteger64Or, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));

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
  cfg.def("set_float", &ConfigWrapper::SetDouble, doc_string.c_str(),
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
  cfg.def("get_float", &ConfigWrapper::GetDouble, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`float` parameter or the default value.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetDoubleOr``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.my-floating-point-number"``.
        default_value: If the parameter does not exist, this value
          will be returned instead.
      )doc";
  cfg.def("get_float_or", &ConfigWrapper::GetDoubleOr, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));

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
      ``werkzeugkiste::config::Configuration::GetStringOr``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"section1.my-str"``.
        default_value: If the parameter does not exist, this value
          will be returned instead.
      )doc";
  cfg.def("get_str_or", &ConfigWrapper::GetStringOr, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));
}

template <typename T>
void GenericScalarSetterUtil(ConfigWrapper &cfg, std::string_view key,
                             T value) {
  if constexpr (std::is_same_v<T, bool>) {
    cfg.SetBoolean(key, value);
  } else if constexpr (std::is_arithmetic_v<T>) {
    constexpr auto value_type = std::is_integral_v<T>
                                    ? wzkcfg::ConfigType::Integer
                                    : wzkcfg::ConfigType::FloatingPoint;
    const auto expected = cfg.Contains(key) ? cfg.Type(key) : value_type;

    if (expected == wzkcfg::ConfigType::Integer) {
      cfg.SetInteger64(key, wzkcfg::CheckedCast<int64_t>(value));
    } else if (expected == wzkcfg::ConfigType::FloatingPoint) {
      cfg.SetDouble(key, wzkcfg::CheckedCast<double>(value));
    } else {
      std::string msg{"Changing the type is not allowed. Parameter `"};
      msg += key;
      msg += "` is `";
      msg += wzkcfg::ToString(expected);
      msg += "`, but input value is of type `";
      msg += wzkcfg::TypeName<T>();
      msg += "!";
      throw wzkcfg::TypeError{msg};
    }
  } else if constexpr (std::is_same_v<T, std::string>) {
    cfg.SetString(key, value);
  } else {
    throw std::runtime_error("Setting this type is not yet supported!");
  }
}

inline void RegisterGenericAccess(pybind11::class_<ConfigWrapper> &cfg) {
  // TODO doc
  cfg.def(
      "__getitem__",
      [cfg](const ConfigWrapper &self,
            const std::string &key) -> pybind11::object {
        switch (self.Type(key)) {
          case wzkcfg::ConfigType::Boolean:
            return pybind11::bool_(self.GetBoolean(key));

          case wzkcfg::ConfigType::Integer:
            return pybind11::int_(self.GetInteger64(key));

          case wzkcfg::ConfigType::FloatingPoint:
            return pybind11::float_(self.GetDouble(key));

          case wzkcfg::ConfigType::String:
            return pybind11::str(self.GetString(key));

            // case wzkcfg::ConfigType::List:
            // return pybind11::list();

          case wzkcfg::ConfigType::Group: {
            pybind11::object obj = cfg();
            auto *ptr = obj.cast<ConfigWrapper *>();
            ptr->ReplaceConfig(self.GetGroup(key));
            return obj;
          }
            // WZKLOG_CRITICAL("TODO not yet implemented!");
            // return pybind11::dict();

          default:
            throw std::runtime_error(
                "Other types (list, date, ...) are not yet implemented!");
        }
        return pybind11::none();
      },
      "Returns the value at the given key (fully-qualified parameter name).",
      pybind11::arg("key"));
  // TODO doc: currently, only scalars
  // type change is not supported
  cfg.def(
      "__setitem__",
      [cfg](ConfigWrapper &self, std::string_view key,
            const pybind11::object &value) -> void {
        if (pybind11::isinstance<pybind11::str>(value)) {
          GenericScalarSetterUtil(self, key, value.cast<std::string>());
        } else if (pybind11::isinstance<pybind11::bool_>(value)) {
          GenericScalarSetterUtil(self, key, value.cast<bool>());
        } else if (pybind11::isinstance<pybind11::int_>(value)) {
          GenericScalarSetterUtil(self, key, value.cast<int64_t>());
        } else if (pybind11::isinstance<pybind11::float_>(value)) {
          GenericScalarSetterUtil(self, key, value.cast<double>());
        } else if (pybind11::isinstance<pybind11::list>(value)) {
          WZKLOG_ERROR("Input value for `{}` is a list - not yet supported",
                       key);
        } else if (pybind11::isinstance<pybind11::dict>(value)) {
          WZKLOG_ERROR("Input value for `{}` is a dict - not yet supported",
                       key);
        } else {
          WZKLOG_ERROR(
              "Input value for `{}` is unknown - need to extend the type check",
              key);
        }
      },
      "Sets the parameter value.", pybind11::arg("key"),
      pybind11::arg("value"));
}

inline void RegisterConfiguration(pybind11::module &m) {
  const std::string module_name = m.attr("__name__").cast<std::string>();
  const std::string config_name = std::string{module_name} + ".Configuration";

  std::string doc_string{};
  std::ostringstream doc_stream;
  // doc_stream
  //     << "TODO A :class:`~" << config_name << "`.\n\n"
  //     << "**Corresponding C++ API:**
  //     ``werkzeugkiste::config::Configuration``.";
  // TODO extend documentation
  doc_string = R"doc(
    Encapsulates parameters.

    This class provides programmable access to a
    `TOML configuration <https://toml.io/en/>`__. It supports
    dictionary-like access to the parameters and provides several
    additional utilities, such as replacing placeholders, adjusting
    relative file paths, merging/nesting configurations, *etc.*

    Provides type-checked access via :meth:`get_int`, :meth:`get_str`,
    *etc.* or allow default values if a *key* does not exist via
    :meth:`get_int_or`, :meth:`get_float_or`, *etc.*
    Parameters can be set via corresponding setters, such as :meth:`set_bool`.
    For convenience, access is also supported via :meth:`__getitem__`
    and :meth:`__setitem__`

    Implicit numeric casts will be performed if the value can be exactly
    represented in the target type. For example, an :class:`int` value
    ``42`` can be exactly represented as :class:`float`, whereas a
    :class:`float` of `0.5` can't be exactly cast to an :class:`int`.
    For the latter, a :class:`~pyzeugkiste.config.TypeError` will be raised.

    .. code-block:: toml
         :caption: Example

         cfg = config.Configuration.load_toml_string("""
             int = 23
             flt = 1.5
             str = "value"
             bool = false
             path = "/home/%USR%/work"
             """)
         cfg['flt']      # Returns a float
         cfg['flt'] = 3  # Parameter is still a float

         cfg['my-str'] = 'value'  # Creates a new string parameter
         cfg.get_str('my_str')    # Alternative access

         cfg.get_int_or('unknown', -1)  # Allow default value

         cfg.replace_placeholders([('%USR%', 'whoami')])

         print(cfg.to_toml_str())
    )doc";

  pybind11::class_<ConfigWrapper> cfg(m, "Configuration", doc_string.c_str());

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

  //---------------------------------------------------------------------------
  // Getter/Setter

  RegisterScalarAccess(cfg);
  RegisterGenericAccess(cfg);

  // TODO size property

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

  cfg.def("empty", &ConfigWrapper::Empty,
          "Checks if this configuration has any parameters set.");

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
  cfg.def("__str__",
          [](const ConfigWrapper &c) {
            //  std::ostringstream s;
            //  s << l;
            //  return s.str();
            return "TODO(ConfigWrapper::ToString)";
          })
      .def("__repr__", [](const ConfigWrapper &c) {
        return "TODO(ConfigWrapper::ToRepr)";
        // std::ostringstream s;
        // s << '<' << l << '>';
        // return s.str();
      });

  cfg.def("__contains__", &ConfigWrapper::Contains,
          "Checks if the given key (fully-qualified parameter name) exists.",
          pybind11::arg("key"));

  // pybind11::register_exception_translator([](std::exception_ptr p) {
  //       try {
  //           if (p) {
  //               std::rethrow_exception(p);
  //           }
  //       } catch (const wzkcfg::KeyError &e) {
  //           PyErr_SetString(PyExc_KeyError, e.what());
  //       } catch (const wzkcfg::TypeError &e) {
  //         PyErr_SetString(PyExc_ValueError, e.what());
  //       } catch (const wzkcfg::ParseError &e) {
  //         PyErr_SetString(PyExc_RuntimeError, e.what());
  //       }
  //   });
  pybind11::register_local_exception<wzkcfg::KeyError>(m, "KeyError",
                                                       PyExc_KeyError);
  pybind11::register_local_exception<wzkcfg::TypeError>(m, "TypeError",
                                                        PyExc_ValueError);
  pybind11::register_local_exception<wzkcfg::ParseError>(m, "ParseError",
                                                         PyExc_RuntimeError);
  // TODO override, or else the stacktrace gets cluttered by the internal
  // C++ module name, e.g. pyzeugkiste._core._cfg.KeyError
  // m.attr("KeyError").attr("__qualname__") = "FIXME";
  m.attr("KeyError").attr("__repr__") = pybind11::cpp_function(
      [](const wzkcfg::KeyError &e) { return "TODO/KeyError"; },
      pybind11::name("__repr__"), pybind11::is_method(m.attr("KeyError")));
  // m.attr("ParseError").def("__repr__", [](const wzkcfg::ParseError &e) {
  //   return "TODO/ParseError";
  // });
}
}  // namespace werkzeugkiste::bindings

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_H
