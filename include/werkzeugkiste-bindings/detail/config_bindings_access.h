#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_ACCESS_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_ACCESS_H

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

// TODO move doc to 'Config'
// inline void RegisterScalarAccess(pybind11::class_<ConfigWrapper> &cfg) {
//   //---------------------------------------------------------------------------
//   // Getting/setting scalars: Boolean
//   std::string doc_string = R"doc(
//       Changes or creates a :class:`bool` parameter.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::SetBoolean``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.subgroup.my-bool"``.
//         value: The value to be set.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
//         a
//           different type (changing the type is not supported); or if the
//           parent path could not be created (*e.g.* if you requested to
//           implicitly create an array).
//       )doc";
//   cfg.def("set_bool", &ConfigWrapper::SetBoolean, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("value"));

//   doc_string = R"doc(
//       Returns the :class:`bool` parameter or raises an exception.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetBoolean``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.subgroup.my-bool"``.

//       Raises:
//         :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a boolean parameter.
//       )doc";
//   cfg.def("get_bool", &ConfigWrapper::GetBoolean, doc_string.c_str(),
//           pybind11::arg("key"));

//   doc_string = R"doc(
//       Returns an optional :class:`bool` parameter or the default value.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetBooleanOr``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.my-bool"``.
//         default_value: If ``key`` does not exist, this value
//           will be returned instead.
//       )doc";
//   cfg.def("get_bool_or", &ConfigWrapper::GetBooleanOr, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("default_value"));

//   //---------------------------------------------------------------------------
//   // Getting/setting scalars: Integer64
//   doc_string = R"doc(
//       Changes or creates an :class:`int` parameter.

//       Note that these bindings assume 64-bit integers, whereas the
//       C++ utility differs between 32- and 64-bit representations.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::SetInteger32`` or
//       ``werkzeugkiste::config::Configuration::SetInteger64``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.subgroup.my-int"``.
//         value: The value to be set.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
//           a different type (changing the type is not supported); or if the
//           parent path could not be created (*e.g.* if you requested to
//           implicitly create an array).
//       )doc";
//   cfg.def("set_int", &ConfigWrapper::SetInteger64, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("value"));

//   doc_string = R"doc(
//       Returns the :class:`int` parameter or raises an exception.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetInteger32`` or
//       ``werkzeugkiste::config::Configuration::GetInteger64``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.subgroup.my-int"``.

//       Raises:
//         :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           an :class:`int` parameter (or cannot be converted to an
//           :class:`int`).
//       )doc";
//   cfg.def("get_int", &ConfigWrapper::GetInteger64, doc_string.c_str(),
//           pybind11::arg("key"));

//   doc_string = R"doc(
//       Returns an optional :class:`int` parameter or the default value.

//       Note that these bindings assume 64-bit integers, whereas the
//       C++ utility differs between 32- and 64-bit representations.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetInteger32Or`` or
//       ``werkzeugkiste::config::Configuration::GetInteger64Or``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.my-integer"``.
//         default_value: If the parameter does not exist, this value
//           will be returned instead.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           an :class:`int` parameter.
//       )doc";
//   cfg.def("get_int_or", &ConfigWrapper::GetInteger64Or, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("default_value"));

//   //---------------------------------------------------------------------------
//   // Getting/setting scalars: Double
//   doc_string = R"doc(
//       Changes or creates a :class:`float` parameter.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::SetDouble``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.subgroup.my-flt"``.
//         value: The value to be set.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
//           a different type (changing the type is not supported); or if the
//           parent path could not be created (*e.g.* if you requested to
//           implicitly create an array).
//       )doc";
//   cfg.def("set_float", &ConfigWrapper::SetDouble, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("value"));

//   doc_string = R"doc(
//       Returns the :class:`float` parameter or raises an exception.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetDouble``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.subgroup.my-flt"``.

//       Raises:
//         :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`float` parameter (or cannot be converted to a
//           :class:`float`).
//       )doc";
//   cfg.def("get_float", &ConfigWrapper::GetDouble, doc_string.c_str(),
//           pybind11::arg("key"));

//   doc_string = R"doc(
//       Returns an optional :class:`float` parameter or the default value.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetDoubleOr``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.my-floating-point-number"``.
//         default_value: If the parameter does not exist, this value
//           will be returned instead.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`float` parameter.
//       )doc";
//   cfg.def("get_float_or", &ConfigWrapper::GetDoubleOr, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("default_value"));

//   //---------------------------------------------------------------------------
//   // Getting/setting scalars: String
//   doc_string = R"doc(
//       Changes or creates a :class:`str` parameter.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::SetString``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.subgroup.my-str"``.
//         value: The value to be set.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
//           a different type (changing the type is not supported); or if the
//           parent path could not be created (*e.g.* if you requested to
//           implicitly create an array).
//       )doc";
//   cfg.def("set_str", &ConfigWrapper::SetString, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("value"));

//   doc_string = R"doc(
//       Returns the :class:`str` parameter or raises an exception.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetString``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.subgroup.my-str"``.

//       Raises:
//         :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`str` parameter.
//       )doc";
//   cfg.def("get_str", &ConfigWrapper::GetString, doc_string.c_str(),
//           pybind11::arg("key"));

//   doc_string = R"doc(
//       Returns an optional :class:`str` parameter or the default value.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetStringOr``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"section1.my-str"``.
//         default_value: If the parameter does not exist, this value
//           will be returned instead.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`str` parameter.
//       )doc";
//   cfg.def("get_str_or", &ConfigWrapper::GetStringOr, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("default_value"));

//   //---------------------------------------------------------------------------
//   // Getting/setting scalars: date
//   doc_string = R"doc(
//       Changes or creates a :class:`datetime.date` parameter.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::SetDate``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.dates.day1"``.
//         value: The :class:`datetime.date` object to be set. Additionally,
//           the value can also be specified as a :class:`str` representation
//           in the format ``Y-m-d`` or ``d.m.Y``. Note that the year component
//           must be :math:`\geq 0`.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
//           a different type (changing the type is not supported); or if the
//           parent path could not be created (*e.g.* if you requested to
//           implicitly create an array).
//       )doc";
//   cfg.def("set_date", &ConfigWrapper::SetDate, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("value"));

//   doc_string = R"doc(
//       Returns the :class:`datetime.date` parameter or raises an exception.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetDate``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.dates.day1"``.

//       Raises:
//         :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`datetime.date` parameter.
//       )doc";
//   cfg.def("get_date", &ConfigWrapper::GetDate, doc_string.c_str(),
//           pybind11::arg("key"));

//   doc_string = R"doc(
//       Returns an optional :class:`datetime.date` parameter or the default
//       value.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetDateOr``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.dates.day1"``.
//         default_value: If the parameter does not exist, this value
//           will be returned instead. See :meth:`set_date` for supported
//           types/representations.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`datetime.date` parameter.
//       )doc";
//   cfg.def("get_date_or", &ConfigWrapper::GetDateOr, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("default_value"));

//   //---------------------------------------------------------------------------
//   // Getting/setting scalars: time

//   doc_string = R"doc(
//       Changes or creates a :class:`datetime.time` parameter.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::SetTime``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.startup_time"``.
//         value: The :class:`datetime.time` object to be set. Additionally,
//           the value can also be specified as a :class:`str` representation
//           in the format ``HH:MM``, ``HH:MM:SS``, ``HH:MM:SS.sss``
//           (milliseconds),
//           ``HH:MM:SS.ssssss`` (microseconds) or ``HH:MM:SS.sssssssss``
//           (nanoseconds). Leap seconds are not supported, *i.e.* the seconds
//           component must be :math:`\in [0, 59]`.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
//           a different type (changing the type is not supported); or if the
//           parent path could not be created (*e.g.* if you requested to
//           implicitly create an array).
//       )doc";
//   cfg.def("set_time", &ConfigWrapper::SetTime, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("value"));

//   doc_string = R"doc(
//       Returns the :class:`datetime.time` parameter or raises an exception.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetTime``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.startup_time"``.

//       Raises:
//         :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`datetime.time` parameter.
//       )doc";
//   cfg.def("get_time", &ConfigWrapper::GetTime, doc_string.c_str(),
//           pybind11::arg("key"));

//   doc_string = R"doc(
//       Returns an optional :class:`datetime.time` parameter or the default
//       value.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetTimeOr``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.startup_time"``.
//         default_value: If the parameter does not exist, this value
//           will be returned instead. See :meth:`set_time` for supported
//           types/representations.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`datetime.time` parameter.
//       )doc";
//   cfg.def("get_time_or", &ConfigWrapper::GetTimeOr, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("default_value"));

//   //---------------------------------------------------------------------------
//   // Getting/setting scalars: datetime

//   doc_string = R"doc(
//       Changes or creates a :class:`datetime.datetime` parameter.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::SetDateTime``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.startup_time"``.
//         value: The :class:`datetime.datetime` object to be set. Additionally,
//           the value can also be specified as a :class:`str` representation
//           in the `RFC 3339 <https://www.rfc-editor.org/rfc/rfc3339>`__
//           format, but the *Unknown Local Offset Convention* (*i.e.*
//           ``-00:00``) and *Leap Seconds* are not supported.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is of
//         a
//           different type (changing the type is not supported); or if the
//           parent path could not be created (*e.g.* if you requested to
//           implicitly create an array).
//       )doc";
//   cfg.def("set_datetime", &ConfigWrapper::SetDateTime, doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("value"));

//   doc_string = R"doc(
//       Returns the :class:`datetime.datetime` parameter or raises an
//       exception.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetDateTime``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.next_run"``.

//       Raises:
//         :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`datetime.datetime` parameter.
//       )doc";
//   cfg.def("get_datetime", &ConfigWrapper::GetDateTime, doc_string.c_str(),
//           pybind11::arg("key"));

//   doc_string = R"doc(
//       Returns an optional :class:`datetime.datetime` parameter or the default
//       value.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::GetDateTimeOr``.

//       Args:
//         key: The fully-qualified parameter name, *e.g.*
//           ``"scheduler.next_run"``.
//         default_value: If the parameter does not exist, this value
//           will be returned instead. See :meth:`set_datetime` for supported
//           types/representations.

//       Raises:
//         :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
//           a :class:`datetime.datetime` parameter.
//       )doc";
//   cfg.def("get_datetime_or", &ConfigWrapper::GetDateTimeOr,
//   doc_string.c_str(),
//           pybind11::arg("key"), pybind11::arg("default_value"));
// }

// inline void RegisterConfigUtilities(pybind11::class_<ConfigWrapper> &cfg) {
//   std::string doc_string = R"doc(
//       Returns the fully-qualified names/keys of **all parameters**.

//       The key defines the "path" from the configuration's root node
//       to the parameter.

//       **Corresponding C++ API:**
//       ``werkzeugkiste::config::Configuration::ListParameterNames``.

//       Args:
//         include_array_entries: If ``True``, the name of each parameter will
//           be returned, *i.e.* the result will also include **each** array
//           element. Otherwise, only explicitly named parameters will be
//           included, refer to the example below.

//       .. code-block:: toml
//          :caption: Exemplary configuration

//          str1 = 'value'

//          [values.numeric]
//          int1 = 42
//          flt1 = 1e-3
//          arr1 = [1, 2, 3]

//          [values.other]
//          str2 = 'value'
//          time1 = 08:00:00
//          arr2 = [
//            'value',
//            { int2 = 123, flt2 = 1.5 }
//          ]

//       .. code-block:: python
//          :caption: Extracted parameter names

//          named_parameters = [  # list_parameter_names(False)
//            'str1',
//            'values',
//            'values.numeric',
//            'values.numeric.arr1',
//            'values.numeric.flt1',
//            'values.numeric.int1',
//            'values.other',
//            'values.other.arr2',
//            'values.other.arr2[1].flt2',
//            'values.other.arr2[1].int2',
//            'values.other.str2',
//            'values.other.time1'
//          ]

//          named_parameters = [  # list_parameter_names(True)
//            'str1',
//            'values',
//            'values.numeric',
//            'values.numeric.arr1',
//            'values.numeric.arr1[0]',
//            'values.numeric.arr1[1]',
//            'values.numeric.arr1[2]',
//            'values.numeric.flt1',
//            'values.numeric.int1',
//            'values.other',
//            'values.other.arr2',
//            'values.other.arr2[0]',
//            'values.other.arr2[1]',
//            'values.other.arr2[1].flt2',
//            'values.other.arr2[1].int2',
//            'values.other.str2',
//            'values.other.time1'
//          ]

//       )doc";
//   cfg.def("list_parameter_names", &ConfigWrapper::ListParameterNames,
//           doc_string.c_str(), pybind11::arg("include_array_entries") =
//           false);

//   // doc_string = R"doc(
//   //     Returns the parameter names/keys of the direct child nodes
//   (first-level
//   //     parameters) of this configuration.

//   //     Returns a :class:`list` of parameter names (*i.e.* a copy), **not**
//   a
//   //     dynamic view. If the configuration changes, any previously returned
//   list
//   //     will **not** be updated automatically.

//   //     To recursively retrieve **all** parameter names within this
//   configuration,
//   //     :meth:`list_parameter_names` should be used instead.

//   //     **Corresponding C++ API:**
//   //     ``werkzeugkiste::config::Configuration::ListParameterNames``.

//   //     .. code-block:: toml
//   //        :caption: Exemplary configuration

//   //        str1 = 'value'

//   //        [values.numeric]
//   //        int1 = 42
//   //        flt1 = 1e-3
//   //        arr1 = [1, 2, 3]

//   //     .. code-block:: python
//   //        :caption: Keys

//   //        keys = cfg.keys()
//   //        # returns ['str1', 'values']

//   //        keys = cfg['values'].keys()
//   //        # returns ['int1', 'flt1', 'arr1']

//   //     )doc";
//   // cfg.def("keys", &ConfigWrapper::Keys, doc_string.c_str());

//   // doc_string = R"doc(
//   //     Replaces **all occurrences** of the given string placeholders.

//   //     Applies string replacement to all :class:`str` parameters of this
//   //     configuration.

//   //     Note:
//   //       The string replacements will be applied in the order specified
//   //       by the ``replacements`` parameter. To avoid any unwanted side
//   effects,
//   //       choose **unique placeholders** that are not contained in any other
//   //       string parameter value or a replacement value.

//   //     **Corresponding C++ API:**
//   //     ``werkzeugkiste::config::Configuration::ReplaceStringPlaceholders``.

//   //     Args:
//   //       replacements: A :class:`list` of ``(search_str, replacement_str)``
//   //         pairs, *i.e.* a :class:`tuple` of :class:`str`.

//   //     Raises:
//   //       :class:`RuntimeError`: If a provided *search_str* is empty.

//   //     Returns:
//   //       ``True`` if any placeholder has actually been replaced.

//   //     .. code-block:: toml
//   //        :caption: Exemplary configuration

//   //        str = 'Release: %VERSION%'
//   //        token = '%TOKEN% - %TOKEN%'

//   //     .. code-block:: python
//   //        :caption: Replace placeholders

//   //        cfg.replace_placeholders([
//   //            ('%VERSION%', 'v0.1'), ('%TOKEN%', '1337')])

//   //     .. code-block:: toml
//   //        :caption: Resulting configuration

//   //        str = 'Release: v0.1'
//   //        token = '1337 - 1337'
//   //     )doc";
//   // cfg.def("replace_placeholders", &ConfigWrapper::ReplacePlaceholders,
//   //         doc_string.c_str(), pybind11::arg("placeholders"));

//   // doc_string = R"doc(
//   //     Loads a nested configuration.

//   //     For example, if the configuration had a field ``"storage"``, which
//   //     should be defined in a separate (*e.g.* machine-dependent)
//   configuration
//   //     file, it could be defined in the main configuration simply
//   //     as ``storage = "path/to/conf.toml"`` or ``storage =
//   "path/to/conf.json"``.

//   //     This function will then load the configuration and replace the
//   //     ``storage`` parameter by the loaded configuration (*i.e.* parameter
//   group).
//   //     Suppose that ``path/to/conf.toml`` defines the parameters
//   //     ``location = ...`` and ``duration = ...``.
//   //     Then, after loading, these parameters can be accessed as
//   //     ``"storage.location"`` and ``"storage.duration"``, respectively.

//   //     **Corresponding C++ API:**
//   //     ``werkzeugkiste::config::Configuration::LoadNestedConfiguration``.

//   //     Raises:
//   //       :class:`~pyzeugkiste.config.ParseError` Upon parsing errors, such
//   as
//   //         file not found, invalid syntax, *etc.*
//   //       :class:`~pyzeugkiste.config.TypeError` If the given parameter is
//   //         not a string.
//   //       :class:`RuntimeError` If the configuration could not be inserted.
//   In
//   //         such cases, please file a bug report.

//   //     Args:
//   //       key: The fully-qualified parameter name which holds the file name
//   //         of the nested `TOML <https://toml.io/en/>`__ configuration (must
//   //         be of type string).
//   //     )doc";
//   // cfg.def("load_nested", &ConfigWrapper::LoadNested, doc_string.c_str(),
//   //         pybind11::arg("key"));

//   // doc_string = R"doc(
//   //     Adjusts string parameters which hold relative file paths.

//   //     After invocation, the given parameters hold either an absolute file
//   path,
//   //     or the concatenation result ``"base_path / <param>"`` if they
//   initially
//   //     held a relative file path.

//   //     To check and adjust such paths, either the fully-qualified names
//   //     of all parameters can be provided, such as ``['file_path',
//   //     'storage.image_path', 'storage.doc_path', ...]``, or a pattern
//   //     which uses the wildcard ``'*'``.
//   //     For example, to adjust **all** parameters which names end with
//   //     the suffix ``_path`` as above, we could simply pass ``['*_path']``.

//   //     Args:
//   //       base_path: Base path to be prepended to relative file paths. Can
//   either
//   //         be a :class:`str` or a :class:`pathlib.Path`.
//   //       parameters: A list of parameter names or patterns.

//   //     Returns:
//   //       ``True`` if any parameter has been adjusted, ``False`` otherwise.

//   //     Raises:
//   //       :class:`~pyzeugkiste.config.TypeError`: If a parameter matches the
//   //         provided names/patterns, but is not a :class:`str` parameter.

//   //     .. code-block:: python
//   //        :caption: Example

//   //        from pyzeugkiste import config

//   //        cfg = config.load_toml_str("""
//   //            file1 = 'rel/path/to/file'
//   //            file2 = '/absolute/path'
//   //            file3 = 'rel/path/to/another/file'

//   //            [output]
//   //            image_folder = 'output/imgs'
//   //            doc_folder = 'output/docs'
//   //            """)

//   //        cfg.adjust_relative_paths(
//   //            'abs-or-rel-path/to/my/workdir',
//   //            ['file*', 'output.*folder'])

//   //        print(cfg.to_toml())
//   //     )doc";
//   // cfg.def("adjust_relative_paths", &ConfigWrapper::AdjustRelativePaths,
//   //         doc_string.c_str(), pybind11::arg("base_path"),
//   //         pybind11::arg("parameters"));
// }

inline void RegisterConfigTypes(pybind11::module &m) {
  //---------------------------------------------------------------------------
  // ConfigType enumeration
  pybind11::enum_<werkzeugkiste::config::ConfigType>(
      m, "ConfigType", "Supported parameter types.")
      .value("Boolean", werkzeugkiste::config::ConfigType::Boolean,
             "A flag, *i.e.* :class:`bool`.")
      .value("Integer", werkzeugkiste::config::ConfigType::Integer,
             "A signed 64-bit integer, *i.e.* :class:`int`.")
      .value("FloatingPoint", werkzeugkiste::config::ConfigType::FloatingPoint,
             "A double-precision floating point number, *i.e.* :class:`float`.")
      .value("String", werkzeugkiste::config::ConfigType::String,
             "A string, *i.e.* :class:`str`.")
      .value("Date", werkzeugkiste::config::ConfigType::Date,
             "A local date, *i.e.* :class:`datetime.date`.")
      .value("Time", werkzeugkiste::config::ConfigType::Time,
             "A local time, *i.e.* :class:`datetime.time`.")
      .value("DateTime", werkzeugkiste::config::ConfigType::DateTime,
             "A date-time following `RFC 3339 "
             "<https://www.rfc-editor.org/rfc/rfc3339>`_, *i.e.* "
             ":class:`datetime.datetime`.")
      .value("List", werkzeugkiste::config::ConfigType::List,
             "A list of unnamed parameters, convertible to :class:`list`.")
      .value("Group", werkzeugkiste::config::ConfigType::Group,
             "A group (collection of named key/value pairs), convertible to "
             ":class:`dict`.");
}

inline void RegisterLoading(pybind11::module &m) {
  //---------------------------------------------------------------------------
  // Loading a configuration

  std::string doc_string = R"doc(
    Loads a configuration file.

    The configuration type will be deduced from the file extension, *i.e.*
    `.toml`, `.json`, or `.cfg`. For JSON files, the default
    :class:`NullValuePolicy` will be used, see :meth:`load_json_file`.

    Args:
      filename: Path to the configuration file, can either be a :class:`str` or
        any object that can be represented as a :class:`str`. For example, a
        :class:`pathlib.Path` is also a valid input parameter.

    Raises:
      :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured, *e.g.* the
          file does not exist, the configuration type cannot be deduced, there are
          syntax errors in the file, *etc.*
  )doc";
  m.def("load", &Config::LoadFile, doc_string.c_str(),
        pybind11::arg("filename"));

  // TODO document exceptions
  m.def("load_toml_str", &Config::LoadTOMLString,
        "Loads the configuration from a `TOML <https://toml.io/en/>`__ string.",
        pybind11::arg("toml_str"));

  m.def("load_toml_file", &Config::LoadTOMLFile,
        "Loads the configuration from a `TOML <https://toml.io/en/>`__ file.",
        pybind11::arg("filename"));

  // TODO NullValuePolicy
  m.def(
      "load_json_str", &Config::LoadJSONString,
      "Loads the configuration from a `JSON <https://www.json.org/>`__ string.",
      pybind11::arg("json_str"));

  // TODO NullValuePolicy
  m.def("load_json_file", &Config::LoadJSONFile,
        "Loads the configuration from a `JSON <https://www.json.org/>`__ file.",
        pybind11::arg("filename"));

  // TODO libconfig (doc: only available if built with libconfig support)
  // TODO enable automatically if libconfig++ is installed or enable via
  //   install extras?
  m.def("load_libconfig_str", &Config::LoadLibconfigString,
        "Loads the configuration from a `Libconfig "
        "<http://hyperrealm.github.io/libconfig/>`__ "
        "string if `libconfig++` is available.",
        pybind11::arg("cfg_str"));

  m.def("load_libconfig_file", &Config::LoadLibconfigFile,
        "Loads the configuration from a `Libconfig "
        "<http://hyperrealm.github.io/libconfig/>`__ file.",
        pybind11::arg("filename"));
}

inline void RegisterBasicOperators(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // General utils/operators
  std::string doc_string = R"doc(
      Checks for equality.

      .. code-block:: python
         :caption: Equality checks

         from pyzeugkiste import config as pyc
         cfg1 = pyc.load_toml_str("""
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
             """)

         # Views/References to sub-groups equal:
         cfg2 = cfg1['values.numeric']
         assert cfg1 != cfg2
         assert cfg1['values.numeric'] == cfg2
         assert cfg1['values']['numeric'] == cfg2

         # Similarly, a deep copy of a sub-group view is also equal:
         cfg2 = cfg1['values.numeric'].copy()
         assert cfg1 != cfg2
         assert cfg1['values.numeric'] == cfg2
         assert cfg1['values']['numeric'] == cfg2

         # Change cfg2 to show that it's really a deep copy:
         cfg2['int1'] = 17
         assert cfg1['values.numeric'] != cfg2
         assert cfg1['values']['numeric'] != cfg2

      Returns:
        ``True`` if both configs contain the exact same configuration,
        *i.e.* keys, corresponding data types and values.
      )doc";
  wrapper.def("__eq__", &Config::Equals, doc_string.c_str(),
              pybind11::arg("other"));

  wrapper.def(
      "__ne__",
      [](const Config &a, const Config &b) -> bool { return !a.Equals(b); },
      "Checks for inequality, see :meth:`__eq__` for details.",
      pybind11::arg("other"));

  wrapper.def("copy", &Config::Copy, "Returns a deep copy.");

  wrapper.def("__contains__", &Config::Contains,
              "Checks if the given key/parameter name exists.",
              pybind11::arg("key"));

  wrapper.def("parameter_len", &Config::ParameterLength,
              "Returns the number of child parameters for the given key.",
              pybind11::arg("key"));

  wrapper.def("__len__", &Config::Length,
              "Returns the number of parameters, *i.e.* direct children of "
              "this configuration.");

  wrapper.def("empty", &Config::Empty,
              "Checks if this configuration contains any parameters.");

  doc_string = R"doc(
      Returns the parameter type.

      Args:
        key: The fully-qualified parameter name. If an empty :class:`str` is
          provided, the type of the current configuration (either
          :class:`~pyzeugkiste.config.ConfigType.Group` or
          :class:`~pyzeugkiste.config.ConfigType.List`) will be
          returned.
      )doc";
  wrapper.def("type", &Config::ParameterType, doc_string.c_str(),
              pybind11::arg("key") = std::string{});

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

      .. code-block:: python
         :caption: Keys

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
              str1 = 'value'

              [values.numeric]
              int1 = 42
              flt1 = 1e-3
              arr1 = [1, 2, 3]
              """)

         keys = cfg.keys()
         # Returns ['str1', 'values']

         keys = cfg['values']['numeric'].keys()
         keys = cfg['values.numeric'].keys()
         # Returns ['int1', 'flt1', 'arr1']

      )doc";
  wrapper.def("keys", &Config::Keys, doc_string.c_str());

  // TODO clear (clears a list or all child parameters of a group)
}

inline void RegisterSerialization(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // Serialization
  wrapper.def("to_toml", &Config::ToTOMLString,
              "Returns a `TOML <https://toml.io/>`__-formatted representation "
              "of this configuration.");

  wrapper.def(
      "to_json", &Config::ToJSONString,
      "Returns a `JSON <https://www.json.org/>`__-formatted representation "
      "of this configuration.\n\nNote that date/time parameters will be "
      "replaced by their string representation.");

  wrapper.def("to_libconfig", &Config::ToLibconfigString,
              "Returns a `Libconfig "
              "<http://hyperrealm.github.io/libconfig/>`__-formatted "
              "representation of this configuration.");

  wrapper.def("to_yaml", &Config::ToYAMLString,
              "Returns a `YAML <https://yaml.org/>`__-formatted representation "
              "of this configuration.");
  // TODO state currently YAML supported version + auto-replacements (date?)

  wrapper.def(
      "to_dict", &Config::ToDict,
      "Returns a :class:`dict` holding all parameters of this configuration.");

  // Representation
  wrapper
      .def("__str__",
           [wrapper](const Config &c) {
             std::ostringstream s;
             const std::string modname =
                 wrapper.attr("__module__").cast<std::string>();
             if (!modname.empty()) {
               s << modname << '.';
             }

             const std::size_t sz = c.Length();
             const auto type = c.Type();
             s << "Config("
               << ((type == werkzeugkiste::config::ConfigType::Group) ? "Group"
                                                                      : "List")
               << " of " << sz << ((sz == 1) ? " parameter" : " parameters")
               << ')';
             return s.str();
           })
      // TODO implement a suitable __repr__
      .def(
          "__repr__", [](const Config &c) { return c.ToTOMLString(); },
          "Returns the TOML string representation such that this configuration "
          "can be reconstructed.");
}

inline void RegisterGenericAccess(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // Generic access
  std::string doc_string;

  // TODO docstr
  wrapper.def(
      "__getitem__",
      [wrapper](Config &c, std::string_view key) {
        return c.GetKey(key, wrapper);
      },
      "", pybind11::arg("key"));
  // TODO docstr
  wrapper.def(
      "__getitem__",
      [wrapper](Config &c, int index) { return c.GetIndex(index, wrapper); },
      "", pybind11::arg("index"));

  // TODO docstr
  wrapper.def("__setitem__", &Config::SetKey, "", pybind11::arg("key"),
              pybind11::arg("value"));
  // TODO docstr
  wrapper.def("__setitem__", &Config::SetIndex, "", pybind11::arg("index"),
              pybind11::arg("value"));

  doc_string = R"doc(
      Deletes a parameter.

      Can be used to delete scalars, lists or sub-groups.
      Deletion of a single list element, however, is not supported.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::Delete``.

      .. code-block:: python
         :caption: Keys

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
             str = 'value'
             int = 42
             flt = 1e-3
             arr = [1, 2, 3]
             """)

         keys = cfg.keys()
         # returns ['arr', 'flt', 'int', 'str']

         del cfg['int']
         keys = cfg.keys()
         # returns ['arr', 'flt', 'str']

         # List elements cannot be deleted:
         del cfg['arr[0]']  # Raises a pyc.KeyError
         del cfg['arr'][0]  # Raises a TypeError (due to the python bindings)

      )doc";
  wrapper.def(
      "__delitem__",
      [](Config &self, std::string_view key) -> void { self.Delete(key); },
      doc_string.c_str(), pybind11::arg("key"));
}

inline void RegisterTypedAccess(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // Typed queries
  std::string doc_string;
  // TODO docstr
  wrapper.def("bool", &Config::GetBool, "", pybind11::arg("key"));
  // TODO docstr
  wrapper.def("bool_or", &Config::GetBoolOr, "", pybind11::arg("key"),
              pybind11::arg("value"));

  // TODO docstr
  wrapper.def("int", &Config::GetInt, "", pybind11::arg("key"));
  // TODO docstr
  wrapper.def("int_or", &Config::GetIntOr, "", pybind11::arg("key"),
              pybind11::arg("value"));

  // TODO docstr
  wrapper.def("float", &Config::GetFloat, "", pybind11::arg("key"));
  // TODO docstr
  wrapper.def("float_or", &Config::GetFloatOr, "", pybind11::arg("key"),
              pybind11::arg("value"));

  // TODO docstr
  wrapper.def("str", &Config::GetStr, "", pybind11::arg("key"));
  // TODO docstr
  wrapper.def("str_or", &Config::GetStrOr, "", pybind11::arg("key"),
              pybind11::arg("value"));

  // TODO docstr
  wrapper.def("date", &Config::GetDate, "", pybind11::arg("key"));
  // TODO docstr
  wrapper.def("date_or", &Config::GetDateOr, "", pybind11::arg("key"),
              pybind11::arg("value"));

  // TODO docstr
  wrapper.def("time", &Config::GetTime, "", pybind11::arg("key"));
  // TODO docstr
  wrapper.def("time_or", &Config::GetTimeOr, "", pybind11::arg("key"),
              pybind11::arg("value"));

  // TODO docstr
  wrapper.def("datetime", &Config::GetDateTime, "", pybind11::arg("key"));
  // TODO docstr
  wrapper.def("datetime_or", &Config::GetDateTimeOr, "", pybind11::arg("key"),
              pybind11::arg("value"));

  doc_string = R"doc(
      Returns a copy of the list parameter as :class:`list`.

      If the list parameter contains any nested :class:`Config`
      instances, they will be converted to :class:`dict`.

      .. code-block:: python
         :caption: Retrieve a deeply copied list

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
             str = 'value'
             lst1 = [1, 2, 3]
             lst2 = [
                 { name = 'test', value = 42 },
                 23
             ]
             """)

         cfg.list('lst1')
         cfg['lst1'].list()
         # Returns [1, 2, 3]

         cfg.list('lst2')
         cfg['lst2'].list()
         # Returns [{'name': 'test', 'value': 42}, 23]

      Args:
        key: If a non-empty :class:`str` is provided, it is interpreted as the
          fully-qualified parameter name of a list parameter. Otherwise,
          ``self`` must be a view of an existing list parameter.
      )doc";
  wrapper.def("list", &Config::GetList, doc_string.c_str(),
              pybind11::arg("key") = std::string{});

  doc_string = R"doc(
      Returns a copied list or the given value if the parameter does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError` If the parameter exists, but is
          not a list.

      Args:
        key: Fully-qualified parameter name of a list parameter. If an
          empty :class:`str` is provided, ``self`` must be a view of an
          existing list parameter.
        value: Any object to be returned if the given ``key`` does not exist.
      )doc";
  wrapper.def("list_or", &Config::GetListOr, doc_string.c_str(),
              pybind11::arg("key"), pybind11::arg("value"));

  // TODO docstr
  wrapper.def("dict", &Config::GetDict, "",
              pybind11::arg("key") = std::string{});
  // TODO docstr
  wrapper.def("dict_or", &Config::GetDictOr, "", pybind11::arg("key"),
              pybind11::arg("value"));
}

inline void RegisterExtendedUtils(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // Special utils
  std::string doc_string = R"doc(
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
  wrapper.def("load_nested", &Config::LoadNested, doc_string.c_str(),
              pybind11::arg("key"));

  doc_string = R"doc(
      Adjusts string parameters which hold relative file paths.

      After invocation, the given parameters hold either an absolute file path,
      or the concatenation result ``"base_path / <param>"`` if they initially
      held a relative file path.

      To check and adjust such paths, either the fully-qualified names
      of all parameters can be provided, such as
      ``['file_path', 'storage.image_path', 'storage.doc_path', ...]``, or a
      pattern which uses the wildcard ``'*'``.
      For example, to adjust **all** parameters which names end with
      the suffix ``_path`` as above, we could simply pass ``['*_path']``.

      Args:
        base_path: Base path to be prepended to relative file paths. Can either
          be a :class:`str` or a :class:`pathlib.Path`.
        parameters: A list of parameter names or patterns.
        key: If a non-empty :class:`str` is provided, it is interpreted as the
          fully-qualified parameter name of a sub-group. Only matching
          parameters below this sub-group will be adjusted.

      Returns:
        ``True`` if any parameter has been adjusted, ``False`` otherwise.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If a parameter matches the
          provided names/patterns, but is not a :class:`str` parameter.
        :class:`~pyzeugkiste.config.KeyError`: If a sub-group ``key`` was
          specified but does not exist.

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
  wrapper.def("adjust_relative_paths", &Config::AdjustRelativePaths,
              doc_string.c_str(), pybind11::arg("base_path"),
              pybind11::arg("parameters"),
              pybind11::arg("key") = std::string{});

  doc_string = R"doc(
      Replaces all occurrences of the given string placeholders.

      Applies string replacement to all :class:`str` parameters of this
      configuration (or a sub-group if ``key`` is specified).

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
        key: If a non-empty :class:`str` is provided, it is interpreted as the
          fully-qualified parameter name of a sub-group. Replacements will only
          affect parameters contained in this sub-group.

      Raises:
        :class:`RuntimeError`: If a provided *search_str* is empty.
        :class:`~pyzeugkiste.config.KeyError`: If a sub-group ``key`` was
          specified but does not exist.

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
  wrapper.def("replace_placeholders", &Config::ReplacePlaceholders,
              doc_string.c_str(), pybind11::arg("placeholders"),
              pybind11::arg("key") = std::string{});

  doc_string = R"doc(
      Returns the fully-qualified names/keys of all parameters below
      the root or specified sub-group.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::ListParameterNames``.

      Args:
        include_array_entries: If ``True``, the name of each parameter will
          be returned, *i.e.* the result will also include **each** array
          element. Otherwise, only explicitly named parameters will be
          included, refer to the example below.
        key: If a non-empty :class:`str` is provided, it is interpreted as the
          fully-qualified parameter name of a sub-group. Replacements will only
          affect parameters contained in this sub-group.

      .. code-block:: python
         :caption: List parameter names

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
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
             """)
         param_names = cfg.list_parameter_names()
         # Returns [
         #   'str1',
         #   'values',
         #   'values.numeric',
         #   'values.numeric.arr1',
         #   'values.numeric.flt1',
         #   'values.numeric.int1',
         #   'values.other',
         #   'values.other.arr2',
         #   'values.other.arr2[1].flt2',
         #   'values.other.arr2[1].int2',
         #   'values.other.str2',
         #   'values.other.time1'
         # ]

         param_names = cfg.list_parameter_names(
             include_array_entries = True)
         # Returns [
         #   'str1',
         #   'values',
         #   'values.numeric',
         #   'values.numeric.arr1',
         #   'values.numeric.arr1[0]',
         #   'values.numeric.arr1[1]',
         #   'values.numeric.arr1[2]',
         #   'values.numeric.flt1',
         #   'values.numeric.int1',
         #   'values.other',
         #   'values.other.arr2',
         #   'values.other.arr2[0]',
         #   'values.other.arr2[1]',
         #   'values.other.arr2[1].flt2',
         #   'values.other.arr2[1].int2',
         #   'values.other.str2',
         #   'values.other.time1'
         # ]

         param_names = cfg.list_parameter_names(key = 'values.numeric')
         param_names = cfg['values']['numeric'].list_parameter_names()
         param_names = cfg['values.numeric'].list_parameter_names()
         # Returns [
         #   'arr1',
         #   'flt1',
         #   'int1'
         # ]

      )doc";
  wrapper.def("list_parameter_names", &Config::ListParameterNames,
              doc_string.c_str(),
              pybind11::arg("include_array_entries") = false,
              pybind11::arg("key") = std::string{});
}
}  // namespace werkzeugkiste::bindings::detail

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_ACCESS_H
