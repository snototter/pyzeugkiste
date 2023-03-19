#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H

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

namespace werkzeugkiste::bindings::detail {
template <typename T>
T ExtractPythonNumber(pybind11::handle obj, const std::string &err_msg) {
  if (pybind11::isinstance<pybind11::int_>(obj)) {
    return werkzeugkiste::config::checked_numcast<T>(obj.cast<int64_t>());
  }

  if (pybind11::isinstance<pybind11::float_>(obj)) {
    return werkzeugkiste::config::checked_numcast<T>(obj.cast<double>());
  }

  throw werkzeugkiste::config::TypeError(err_msg);
}

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
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of a
          different type (changing the type is not supported); or if the parent
          path could not be created (*e.g.* if you requested to implicitly
          create an array).
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
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a boolean parameter.
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
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
          a different type (changing the type is not supported); or if the
          parent path could not be created (*e.g.* if you requested to
          implicitly create an array).
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
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          an :class:`int` parameter (or cannot be converted to an :class:`int`).
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

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          an :class:`int` parameter.
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
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
          a different type (changing the type is not supported); or if the
          parent path could not be created (*e.g.* if you requested to
          implicitly create an array).
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
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`float` parameter (or cannot be converted to a :class:`float`).
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

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`float` parameter.
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
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
          a different type (changing the type is not supported); or if the
          parent path could not be created (*e.g.* if you requested to
          implicitly create an array).
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
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`str` parameter.
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

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`str` parameter.
      )doc";
  cfg.def("get_str_or", &ConfigWrapper::GetStringOr, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));

  //---------------------------------------------------------------------------
  // Getting/setting scalars: date
  doc_string = R"doc(
      Changes or creates a :class:`datetime.date` parameter.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::SetDate``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.dates.day1"``.
        value: The :class:`datetime.date` object to be set. Additionally,
          the value can also be specified as a :class:`str` representation
          in the format ``Y-m-d`` or ``d.m.Y``. Note that the year component
          must be :math:`\geq 0`.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
          a different type (changing the type is not supported); or if the
          parent path could not be created (*e.g.* if you requested to
          implicitly create an array).
      )doc";
  cfg.def("set_date", &ConfigWrapper::SetDate, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`datetime.date` parameter or raises an exception.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetDate``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.dates.day1"``.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.date` parameter.
      )doc";
  cfg.def("get_date", &ConfigWrapper::GetDate, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`datetime.date` parameter or the default value.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetDateOr``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.dates.day1"``.
        default_value: If the parameter does not exist, this value
          will be returned instead. See :meth:`set_date` for supported
          types/representations.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.date` parameter.
      )doc";
  cfg.def("get_date_or", &ConfigWrapper::GetDateOr, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));

  //---------------------------------------------------------------------------
  // Getting/setting scalars: time

  doc_string = R"doc(
      Changes or creates a :class:`datetime.time` parameter.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::SetTime``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.startup_time"``.
        value: The :class:`datetime.time` object to be set. Additionally,
          the value can also be specified as a :class:`str` representation
          in the format ``HH:MM``, ``HH:MM:SS``, ``HH:MM:SS.sss`` (milliseconds),
          ``HH:MM:SS.ssssss`` (microseconds) or ``HH:MM:SS.sssssssss`` (nanoseconds).
          Leap seconds are not supported, *i.e.* the seconds component must
          be :math:`\in [0, 59]`.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
          a different type (changing the type is not supported); or if the
          parent path could not be created (*e.g.* if you requested to
          implicitly create an array).
      )doc";
  cfg.def("set_time", &ConfigWrapper::SetTime, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`datetime.time` parameter or raises an exception.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetTime``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.startup_time"``.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.time` parameter.
      )doc";
  cfg.def("get_time", &ConfigWrapper::GetTime, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`datetime.time` parameter or the default value.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetTimeOr``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.startup_time"``.
        default_value: If the parameter does not exist, this value
          will be returned instead. See :meth:`set_time` for supported
          types/representations.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.time` parameter.
      )doc";
  cfg.def("get_time_or", &ConfigWrapper::GetTimeOr, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));

  //---------------------------------------------------------------------------
  // Getting/setting scalars: datetime

  doc_string = R"doc(
      Changes or creates a :class:`datetime.datetime` parameter.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::SetDateTime``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.startup_time"``.
        value: The :class:`datetime.datetime` object to be set. Additionally,
          the value can also be specified as a :class:`str` representation
          in the `RFC 3339 <https://www.rfc-editor.org/rfc/rfc3339>`__ format,
          but the *Unknown Local Offset Convention* (*i.e.* ``-00:00``) and
          *Leap Seconds* are not supported.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of a
          different type (changing the type is not supported); or if the parent
          path could not be created (*e.g.* if you requested to implicitly
          create an array).
      )doc";
  cfg.def("set_datetime", &ConfigWrapper::SetDateTime, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`datetime.datetime` parameter or raises an exception.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetDateTime``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.next_run"``.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.datetime` parameter.
      )doc";
  cfg.def("get_datetime", &ConfigWrapper::GetDateTime, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`datetime.datetime` parameter or the default value.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::GetDateTimeOr``.

      Args:
        key: The fully-qualified parameter name, *e.g.*
          ``"scheduler.next_run"``.
        default_value: If the parameter does not exist, this value
          will be returned instead. See :meth:`set_datetime` for supported
          types/representations.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.datetime` parameter.
      )doc";
  cfg.def("get_datetime_or", &ConfigWrapper::GetDateTimeOr, doc_string.c_str(),
          pybind11::arg("key"), pybind11::arg("default_value"));
}

template <typename TSet, typename TIn>
void GenericScalarSetterUtil(ConfigWrapper &cfg, std::string_view key,
                             TIn value) {
  if constexpr (std::is_same_v<TSet, bool>) {
    static_assert(std::is_same_v<TIn, bool>);
    cfg.SetBoolean(key, value);
  } else if constexpr (std::is_arithmetic_v<TSet> &&
                       !std::is_same_v<TSet, bool>) {
    static_assert(std::is_arithmetic_v<TIn>);
    constexpr auto value_type = std::is_integral_v<TIn>
                                    ? werkzeugkiste::config::ConfigType::Integer
                                    : werkzeugkiste::config::ConfigType::FloatingPoint;
    const auto expected = cfg.Contains(key) ? cfg.Type(key) : value_type;

    if (expected == werkzeugkiste::config::ConfigType::Integer) {
      cfg.SetInteger64(key, werkzeugkiste::config::checked_numcast<int64_t>(value));
    } else if (expected == werkzeugkiste::config::ConfigType::FloatingPoint) {
      cfg.SetDouble(key, werkzeugkiste::config::checked_numcast<double>(value));
    } else {
      std::string msg{"Changing the type is not allowed. Parameter `"};
      msg += key;
      msg += "` is `";
      msg += werkzeugkiste::config::ConfigTypeToString(expected);
      msg += "`, but input value is of type `";
      msg += werkzeugkiste::config::TypeName<TIn>();
      msg += "!";
      throw werkzeugkiste::config::TypeError{msg};
    }
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
    std::string msg{"Type `"};
    msg += werkzeugkiste::config::TypeName<TSet>;
    msg += "` is not supported in GenericScalarSetterUtil!";
    throw std::logic_error{msg};
  }
}

inline void RegisterGenericAccess(pybind11::class_<ConfigWrapper> &cfg) {
  std::string doc{R"doc(
    Returns a **copy** of the parameter.

    Args:
      key: Fully-qualified parameter name as :class:`str`.
    
    Returns:
      A **copy** of the parameter.
    )doc"};
  cfg.def(
      "__getitem__",
      [cfg](const ConfigWrapper &self,
            const std::string &key) -> pybind11::object {
        switch (self.Type(key)) {
          case werkzeugkiste::config::ConfigType::Boolean:
            return pybind11::bool_(self.GetBoolean(key));

          case werkzeugkiste::config::ConfigType::Integer:
            return pybind11::int_(self.GetInteger64(key));

          case werkzeugkiste::config::ConfigType::FloatingPoint:
            return pybind11::float_(self.GetDouble(key));

          case werkzeugkiste::config::ConfigType::String:
            return pybind11::str(self.GetString(key));

          case werkzeugkiste::config::ConfigType::List:
            return ListToPyList(&cfg, self.GetRootGroup(), key);

          case werkzeugkiste::config::ConfigType::Group:
            return GroupToPyObj(&cfg, self.GetGroup(key));

          case werkzeugkiste::config::ConfigType::Date:
            return self.GetDate(key);

          case werkzeugkiste::config::ConfigType::Time:
            return self.GetTime(key);

          case werkzeugkiste::config::ConfigType::DateTime:
            return self.GetDateTime(key);
        }
        return pybind11::none();
      },
      doc.c_str(), pybind11::arg("key"));

  // TODO doc: type change is not supported (except for lists)
  cfg.def(
      "__setitem__",
      [cfg](ConfigWrapper &self, std::string_view key,
            const pybind11::object &value) -> void {
        const std::string tp = pybind11::cast<std::string>(
            value.attr("__class__").attr("__name__"));

        if (self.Contains(key)) {
          std::string err_msg{"Cannot use a python object of type `"};
          err_msg += tp;
          err_msg += "` to update existing parameter `";
          err_msg += key;
          err_msg += "`!";

          // Existing parameter defines which type to insert:
          switch (self.Type(key)) {
            case werkzeugkiste::config::ConfigType::Boolean:
              if (pybind11::isinstance<pybind11::bool_>(value)) {
                GenericScalarSetterUtil<bool>(self, key, value.cast<bool>());
                return;
              }
              break;

            case werkzeugkiste::config::ConfigType::Integer:
              GenericScalarSetterUtil<int64_t>(
                  self, key, ExtractPythonNumber<int64_t>(value, err_msg));
              return;

            case werkzeugkiste::config::ConfigType::FloatingPoint:
              GenericScalarSetterUtil<double>(
                  self, key, ExtractPythonNumber<double>(value, err_msg));
              return;

            case werkzeugkiste::config::ConfigType::String:
              if (pybind11::isinstance<pybind11::str>(value)) {
                GenericScalarSetterUtil<std::string>(self, key,
                                                     value.cast<std::string>());
                return;
              }
              break;

            case werkzeugkiste::config::ConfigType::List:
              if (pybind11::isinstance<pybind11::list>(value) ||
                  pybind11::isinstance<pybind11::tuple>(value)) {
                self.SetList(key, value);
                return;
              }
              break;

            case werkzeugkiste::config::ConfigType::Group:
              if (pybind11::isinstance<pybind11::dict>(value)) {
                self.SetGroup(key, ConfigWrapper::FromPyDict(
                                       value.cast<pybind11::dict>()));
              } else if (pybind11::isinstance<ConfigWrapper>(value)) {
                self.SetGroup(key, value.cast<ConfigWrapper>());
                return;
              }
              break;

            case werkzeugkiste::config::ConfigType::Date:
              GenericScalarSetterUtil<werkzeugkiste::config::date>(self, key, value);
              return;

            case werkzeugkiste::config::ConfigType::Time:
              GenericScalarSetterUtil<werkzeugkiste::config::time>(self, key, value);
              return;

            case werkzeugkiste::config::ConfigType::DateTime:
              GenericScalarSetterUtil<werkzeugkiste::config::date_time>(self, key, value);
              return;
          }
          throw werkzeugkiste::config::TypeError(err_msg);
        } else {
          // Python type defines what kind of parameter to insert:
          if (pybind11::isinstance<pybind11::str>(value)) {
            GenericScalarSetterUtil<std::string>(self, key,
                                                 value.cast<std::string>());
          } else if (pybind11::isinstance<pybind11::bool_>(value)) {
            GenericScalarSetterUtil<bool>(self, key, value.cast<bool>());
          } else if (pybind11::isinstance<pybind11::int_>(value)) {
            GenericScalarSetterUtil<int64_t>(self, key, value.cast<int64_t>());
          } else if (pybind11::isinstance<pybind11::float_>(value)) {
            GenericScalarSetterUtil<double>(self, key, value.cast<double>());
          } else if (pybind11::isinstance<pybind11::list>(value) ||
                     pybind11::isinstance<pybind11::tuple>(value)) {
            self.SetList(key, value);
          } else if (pybind11::isinstance<pybind11::dict>(value)) {
            self.SetGroup(
                key, ConfigWrapper::FromPyDict(value.cast<pybind11::dict>()));
          } else if (pybind11::isinstance<ConfigWrapper>(value)) {
            self.SetGroup(key, value.cast<ConfigWrapper>());
          } else {
            if (tp.compare("date") == 0) {
              GenericScalarSetterUtil<werkzeugkiste::config::date>(self, key, value);
            } else if (tp.compare("time") == 0) {
              GenericScalarSetterUtil<werkzeugkiste::config::time>(self, key, value);
            } else if (tp.compare("datetime") == 0) {
              GenericScalarSetterUtil<werkzeugkiste::config::date_time>(self, key, value);
            } else {
              std::string msg{"Creating a new parameter (at key `"};
              msg += key;
              msg += "`) from python type `";
              msg += tp;
              msg += "` is not supported!";

              throw werkzeugkiste::config::TypeError(msg);
            }
          }
        }
      },
      "Sets the parameter value.", pybind11::arg("key"),
      pybind11::arg("value"));

  // TODO raises KeyError
  // list items cannot be deleted, only "full" (named) parameters
  cfg.def(
      "__delitem__",
      [cfg](ConfigWrapper &self, std::string_view key) -> void {
        self.Delete(key);
      },
      "Deletes a parameter.", pybind11::arg("key"));
}

inline void RegisterConfigUtilities(pybind11::class_<ConfigWrapper> &cfg) {
  std::string doc_string = R"doc(
      Returns the fully-qualified names/keys of **all parameters**.

      The key defines the "path" from the configuration's root node
      to the parameter.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::ListParameterNames``.

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
      Returns the parameter names/keys of the direct child nodes (first-level
      parameters) of this configuration.

      Returns a :class:`list` of parameter names (*i.e.* a copy), **not** a
      dynamic view. If the configuration changes, any previously returned list
      will **not** be updated automatically.

      To recursively retrieve **all** parameter names within this configuration,
      :meth:`list_parameter_names` should be used instead.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::ListParameterNames``.

      .. code-block:: toml
         :caption: Exemplary configuration

         str1 = 'value'

         [values.numeric]
         int1 = 42
         flt1 = 1e-3
         arr1 = [1, 2, 3]

      .. code-block:: python
         :caption: Keys

         keys = cfg.keys()
         # returns ['str1', 'values']

         keys = cfg['values'].keys()
         # returns ['int1', 'flt1', 'arr1']

      )doc";
  cfg.def("keys", &ConfigWrapper::Keys, doc_string.c_str());

  doc_string = R"doc(
      Replaces **all occurrences** of the given string placeholders.

      Applies string replacement to all :class:`str` parameters of this
      configuration.

      Note:
        The string replacements will be applied in the order specified
        by the ``replacements`` parameter. To avoid any unwanted side effects,
        choose **unique placeholders** that are not contained in any other
        string parameter value or a replacement value.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::ReplaceStringPlaceholders``.

      Args:
        replacements: A :class:`list` of ``(search_str, replacement_str)``
          pairs, *i.e.* a :class:`tuple` of :class:`str`.

      Raises:
        :class:`RuntimeError`: If a provided *search_str* is empty.

      Returns:
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

  doc_string = R"doc(
      Loads a nested configuration.

      For example, if the configuration had a field ``"storage"``, which
      should be defined in a separate (*e.g.* machine-dependent) configuration
      file, it could be defined in the main configuration simply
      as ``storage = "path/to/conf.toml"`` or ``storage = "path/to/conf.json"``.

      This function will then load the configuration and replace the
      ``storage`` parameter by the loaded configuration (*i.e.* parameter group).
      Suppose that ``path/to/conf.toml`` defines the parameters
      ``location = ...`` and ``duration = ...``.
      Then, after loading, these parameters can be accessed as
      ``"storage.location"`` and ``"storage.duration"``, respectively.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::LoadNestedConfiguration``.

      Raises:
        :class:`~pyzeugkiste.config.ParseError` Upon parsing errors, such as
          file not found, invalid syntax, *etc.*
        :class:`~pyzeugkiste.config.TypeError` If the given parameter is
          not a string.
        :class:`RuntimeError` If the configuration could not be inserted. In
          such cases, please file a bug report.

      Args:
        key: The fully-qualified parameter name which holds the file name
          of the nested `TOML <https://toml.io/en/>`__ configuration (must
          be of type string).
      )doc";
  cfg.def("load_nested", &ConfigWrapper::LoadNested, doc_string.c_str(),
          pybind11::arg("key"));

  doc_string = R"doc(
      Adjusts string parameters which hold relative file paths.

      After invocation, the given parameters hold either an absolute file path,
      or the concatenation result ``"base_path / <param>"`` if they initially
      held a relative file path.

      To check and adjust such paths, either the fully-qualified names
      of all parameters can be provided, such as ``['file_path',
      'storage.image_path', 'storage.doc_path', ...]``, or a pattern
      which uses the wildcard ``'*'``.
      For example, to adjust **all** parameters which names end with
      the suffix ``_path`` as above, we could simply pass ``['*_path']``.

      Args:
        base_path: Base path to be prepended to relative file paths. Can either
          be a :class:`str` or a :class:`pathlib.Path`.
        parameters: A list of parameter names or patterns.

      Returns:
        ``True`` if any parameter has been adjusted, ``False`` otherwise.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If a parameter matches the
          provided names/patterns, but is not a :class:`str` parameter.

      .. code-block:: python
         :caption: Example

         from pyzeugkiste import config

         cfg = config.load_toml_str("""
             file1 = 'rel/path/to/file'
             file2 = '/absolute/path'
             file3 = 'rel/path/to/another/file'

             [output]
             image_folder = 'output/imgs'
             doc_folder = 'output/docs'
             """)

         cfg.adjust_relative_paths(
             'abs-or-rel-path/to/my/workdir',
             ['file*', 'output.*folder'])

         print(cfg.to_toml())
      )doc";
  cfg.def("adjust_relative_paths", &ConfigWrapper::AdjustRelativePaths,
          doc_string.c_str(), pybind11::arg("base_path"),
          pybind11::arg("parameters"));
}
}  // namespace werkzeugkiste::bindings::detail

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_TYPES_H