#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_H

#include <pybind11/chrono.h>
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

/*
# TODO: make test suite:

from pyzeugkiste import config
import datetime
c = config.load_toml_str("""
    day = 2022-12-01
    time = 08:30:00
    """)
c['day']
c['time']

c['day'] = datetime.date(2020, 10, 20)
c['time'] = datetime.time(10, 8, 30)
c['day']
c['time']

now = datetime.datetime.now()
c['day'] = now  # should fail
c['day'] = now.date()
c['time'] = now  # should fail
c['time'] = now.time()
c['not-yet-supported'] = now
c['day']
c['time']
c['dt'] = now  # should work

import pytz
now_utc = now.astimezone(pytz.utc)

c['utc'] = now_utc
c['utc']

now_est = now.astimezone(pytz.timezone('EST'))
c['est'] = now_est
c['est']
assert c['est'] == c['utc']

assert str(c['est']) != str(now_est)
assert c['est'] == now_est


c['a'] = now.date()
c['b'] = now.time()
print(c.to_toml())


from pyzeugkiste import config

c = config.load_toml_str("""
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

now.utcoffset() -> None
now_utc.utcoffset() -> timedelta


*/

namespace werkzeugkiste::bindings::detail {
// class ConfigWrapper; //TODO remove

class Config;

void RegisterConfigTypes(pybind11::module &m);
void RegisterLoading(pybind11::module &m);
void RegisterBasicOperators(pybind11::class_<Config> &wrapper);
void RegisterSerialization(pybind11::class_<Config> &wrapper);
void RegisterGenericAccess(pybind11::class_<Config> &wrapper);
void RegisterTypedAccess(pybind11::class_<Config> &wrapper);
void RegisterExtendedUtils(pybind11::class_<Config> &wrapper);

std::string PyObjToString(pybind11::handle path);

werkzeugkiste::config::date PyObjToDate(pybind11::handle obj);
werkzeugkiste::config::time PyObjToTime(pybind11::handle obj);
werkzeugkiste::config::date_time PyObjToDateTime(pybind11::handle obj);

pybind11::object DateToPyObj(const werkzeugkiste::config::date &d);
pybind11::object TimeToPyObj(const werkzeugkiste::config::time &t);
pybind11::object DateTimeToPyObj(const werkzeugkiste::config::date_time &dt);

werkzeugkiste::config::Configuration PyDictToConfiguration(
    const pybind11::dict &d);
void ExtractPyIterable(werkzeugkiste::config::Configuration &cfg,
                       std::string_view key, pybind11::handle lst);

// // TODO remove
// pybind11::list ListToPyList(
//     const pybind11::class_<ConfigWrapper> *cls_handle,
//     const werkzeugkiste::config::Configuration &cfg,
//     std::string_view key);
// // TODO remove
// pybind11::object GroupToPyObj(
//     const pybind11::class_<ConfigWrapper> *cls_handle,
//     const werkzeugkiste::config::Configuration &wcfg);
// // TODO remove
// void RegisterScalarAccess(pybind11::class_<ConfigWrapper> &cfg);
// // TODO remove
// void RegisterGenericAccess(pybind11::class_<ConfigWrapper> &cfg);
// // TODO remove
// void RegisterConfigUtilities(pybind11::class_<ConfigWrapper> &cfg);
}  // namespace werkzeugkiste::bindings::detail

#include <werkzeugkiste-bindings/detail/config_bindings_access.h>
#include <werkzeugkiste-bindings/detail/config_bindings_types.h>

namespace werkzeugkiste::bindings {
inline void RegisterConfigUtils(pybind11::module &main_module) {
  pybind11::module m = main_module.def_submodule("_cfg");
  m.doc() = R"doc(
    Configuration file utils.

    TODO summary
    )doc";

  // const std::string module_name = m.attr("__name__").cast<std::string>(); //
  // TODO check if/where needed const std::string config_name =
  // std::string{module_name} + ".Config";

  detail::RegisterConfigTypes(m);

  std::string doc_string = R"doc(
    Encapsulates parameters.

    This class provides dictionary-like access to parameters and
    provides several additional utilities, such as replacing placeholders, adjusting
    relative file paths, merging/nesting configurations, *etc.*

    This utitility is intended for *"typical"* configuration scenarios. Thus,
    it supports the following basic types: :class:`bool`, :class:`int`,
    :class:`float`, and :class:`str`. As it uses `TOML <https://toml.io/en/>`__
    under the hood, it also supports explicit date and time types.
    Parameters can be combined into a :class:`list`, or into parameter
    groups, which correspond to *tables* in `TOML <https://toml.io/en/>`__,
    *groups* in `libconfig <http://hyperrealm.github.io/libconfig/>`__,
    *objects* in `JSON <https://www.json.org/>`__ or :class:`dict` in python.

    **Type-checked access** is provided via :meth:`get_int`, :meth:`get_str`,
    *etc.* or allow default values if a *key* does not exist via
    :meth:`get_int_or`, :meth:`get_float_or`, *etc.*
    Parameters can be set via corresponding setters, such as :meth:`set_bool`.
    For convenience, access is also supported via :meth:`__getitem__`
    and :meth:`__setitem__`.

    **Implicit numeric casts** will be performed if the value can be **exactly
    represented** in the target type.
    For example, an :class:`int` value 42 can be exactly represented by
    a :class:`float`, whereas a :class:`float` of 0.5 can't be cast to
    an :class:`int`. For the latter cast, a
    :class:`~pyzeugkiste.config.TypeError` would be raised.

    .. code-block:: python
         :caption: Example

         from pyzeugkiste import config as pyc

         cfg = pyc.load_toml_str("""
             int = 23
             flt = 1.5
             str = "value"
             bool = false

             [replacements]
             work_dir = "/home/%USR%/work"
             output_file = "%OUT%/dump.bin"

             [disk]
             network_config = "configs/net.toml"
             model_path = "models/latest.bin"
             image_path = "images/"
             """)

         'str' in cfg    # Returns True

         cfg['flt']      # Returns a float
         cfg['flt'] = 3  # Parameter is still a float

         cfg['my_str'] = 'value'  # Creates a new string parameter
         cfg.str('my_str')        # Alternative access

         cfg.int_or('unknown', -1)  # Returns the fallback/default value

         cfg['unknown']  # Raises a KeyError
         cfg.int('str')  # Raises a TypeError

         cfg.replace_placeholders([
             ('%USR%', 'whoami'),
             ('%OUT%', '/path/to/output')])

         cfg.adjust_relative_paths(
             '/path/to/workdir',
             ['network_config', 'disk.*path'])

         cfg.load_nested('disk.network_config')
         cfg['disk.network_config']  # Is now a group/dictionary

         print(cfg.to_toml())
    )doc";

  // m.def("load_test", &detail::Config::LoadTOMLString, "TODO");
  pybind11::class_<detail::Config> wrapper(m, "Config", doc_string.c_str());
  wrapper.def(pybind11::init<>());

  //---------------------------------------------------------------------------
  // Loading a configuration
  detail::RegisterLoading(m);

  //---------------------------------------------------------------------------
  // Serialization
  detail::RegisterSerialization(wrapper);

  //---------------------------------------------------------------------------
  // General utils/operators
  detail::RegisterBasicOperators(wrapper);

  //---------------------------------------------------------------------------
  // Typed queries (int, int_or, list, list_or, ...)
  detail::RegisterTypedAccess(wrapper);

  //---------------------------------------------------------------------------
  // Generic access (__getitem__, __setitem__, __delitem__)
  detail::RegisterGenericAccess(wrapper);

  //---------------------------------------------------------------------------
  // Special utils
  detail::RegisterExtendedUtils(wrapper);

  // doc_string = R"doc(
  //   Encapsulates parameters.

  //   This class provides dictionary-like access to parameters and
  //   provides several additional utilities, such as replacing placeholders,
  //   adjusting relative file paths, merging/nesting configurations, *etc.*

  //   This utitility is intended for *"typical"* configuration scenarios. Thus,
  //   it supports the following basic types: :class:`bool`, :class:`int`,
  //   :class:`float`, and :class:`str`. As it uses `TOML
  //   <https://toml.io/en/>`__ under the hood, it also supports explicit date
  //   and time types. Parameters can be combined into a :class:`list`, or into
  //   parameter groups, which correspond to *tables* in `TOML
  //   <https://toml.io/en/>`__, *groups* in `libconfig
  //   <http://hyperrealm.github.io/libconfig/>`__, *objects* in `JSON
  //   <https://www.json.org/>`__ or :class:`dict` in python.

  //   **Type-checked access** is provided via :meth:`get_int`, :meth:`get_str`,
  //   *etc.* or allow default values if a *key* does not exist via
  //   :meth:`get_int_or`, :meth:`get_float_or`, *etc.*
  //   Parameters can be set via corresponding setters, such as
  //   :meth:`set_bool`. For convenience, access is also supported via
  //   :meth:`__getitem__` and :meth:`__setitem__`.

  //   **Implicit numeric casts** will be performed if the value can be
  //   **exactly represented** in the target type. For example, an :class:`int`
  //   value 42 can be exactly represented by a :class:`float`, whereas a
  //   :class:`float` of 0.5 can't be cast to an :class:`int`. For the latter
  //   cast, a :class:`~pyzeugkiste.config.TypeError` would be raised.

  //   .. code-block:: python
  //        :caption: Example

  //        from pyzeugkiste import config

  //        cfg = config.load_toml_str("""
  //            int = 23
  //            flt = 1.5
  //            str = "value"
  //            bool = false

  //            [replacements]
  //            work_dir = "/home/%USR%/work"
  //            output_file = "%OUT%/dump.bin"

  //            [disk]
  //            network_config = "configs/net.toml"
  //            model_path = "models/latest.bin"
  //            image_path = "images/"
  //            """)

  //        'str' in cfg    # Returns True

  //        cfg['flt']      # Returns a float
  //        cfg['flt'] = 3  # Parameter is still a float

  //        cfg['my_str'] = 'value'  # Creates a new string parameter
  //        cfg.get_str('my_str')    # Alternative access

  //        cfg.get_int_or('unknown', -1)  # Allow default value

  //        cfg['unknown']      # Raises a KeyError
  //        cfg.get_int('str')  # Raises a TypeError

  //        cfg.replace_placeholders([
  //            ('%USR%', 'whoami'),
  //            ('%OUT%', '/path/to/output')])

  //        cfg.adjust_relative_paths(
  //            '/path/to/workdir',
  //            ['network_config', 'disk.*path'])

  //        cfg.load_nested('disk.network_config')
  //        cfg['disk.network_config']  # Is now a group/dictionary

  //        print(cfg.to_toml())
  //   )doc";

  // pybind11::class_<detail::ConfigWrapper> cfg(m, "Configuration",
  //                                             doc_string.c_str());

  // cfg.def(pybind11::init<>(), "Creates an empty configuration.");

  // //TODO
  // // cfg.def("view", &detail::ConfigWrapper::Get, "TODO");
  // // cfg.def("debug", [](detail::ConfigWrapper &c, std::string_view key) {
  // //   if (c.Type(key) == werkzeugkiste::config::ConfigType::Group) {
  // //     return c.Get(key);
  // //   }
  // //   return pybind11::none();
  // // });

  // //---------------------------------------------------------------------------
  // // General members/operators
  // cfg.def("empty", &detail::ConfigWrapper::Empty,
  //         "Checks if this configuration has any parameters set.");

  // // Equality checks
  // cfg.def(
  //     "__eq__",
  //     [](const detail::ConfigWrapper &a,
  //        const detail::ConfigWrapper &b) -> bool { return a.Equals(b); },
  //     "Checks for equality.\n\nReturns ``True`` if both configs contain the "
  //     "exact same configuration, *i.e.* keys, corresponding data types and\n"
  //     "values.",
  //     pybind11::arg("other"));

  // cfg.def(
  //     "__ne__",
  //     [](const detail::ConfigWrapper &a,
  //        const detail::ConfigWrapper &b) -> bool { return !a.Equals(b); },
  //     "Checks for inequality, see :meth:`__eq__` for details.",
  //     pybind11::arg("other"));

  // cfg.def("__str__",
  //         [cfg](const detail::ConfigWrapper &c) {
  //           std::ostringstream s;
  //           const std::string modname =
  //               cfg.attr("__module__").cast<std::string>();
  //           if (!modname.empty()) {
  //             s << modname << '.';
  //           }

  //           const std::size_t sz = c.Size();
  //           s << "Configuration(" << sz
  //             << ((sz == 1) ? " parameter" : " parameters") << ')';
  //           return s.str();
  //         })
  //     .def("__repr__", [](const detail::ConfigWrapper &c) {
  //       return c.ToTOMLString();
  //       // return c.attr("__name__").cast<std::string>();
  //       //  return cfg.attr("__name__").cast<std::string>();
  //       //  std::ostringstream s;
  //       //  s << m.attr("__name__").cast<std::string>() <<
  //       ".Configuration()";
  //       //  // TODO x parameters, ...
  //       //  return s.str();
  //     });

  // cfg.def("__contains__", &detail::ConfigWrapper::Contains,
  //         "Checks if the given key (fully-qualified parameter name) exists.",
  //         pybind11::arg("key"));

  // cfg.def("__len__", &detail::ConfigWrapper::Size,
  //         "Returns the number of parameters (key-value pairs) in this "
  //         "configuration.");

  // //---------------------------------------------------------------------------
  // // Loading a configuration

  // doc_string = R"doc(
  //   Loads a configuration file.

  //   The configuration type will be deduced from the file extension, *i.e.*
  //   `.toml`, `.json`, or `.cfg`. For JSON files, the default
  //   :class:`NullValuePolicy` will be used, see :meth:`load_json_file`.

  //   Args:
  //     filename: Path to the configuration file, can either be a :class:`str`
  //     or
  //       any object that can be represented as a :class:`str`. For example, a
  //       :class:`pathlib.Path` is also a valid input parameter.

  //   Raises:
  //     :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured,
  //     *e.g.* the
  //         file does not exist, the configuration type cannot be deduced,
  //         there are syntax errors in the file, *etc.*
  // )doc";
  // m.def("load", &detail::ConfigWrapper::LoadFile, doc_string.c_str(),
  //       pybind11::arg("filename"));

  // // TODO document exceptions
  // m.def("load_toml_str", &detail::ConfigWrapper::LoadTOMLString,
  //       "Loads the configuration from a `TOML <https://toml.io/en/>`__
  //       string.", pybind11::arg("toml_str"));

  // m.def("load_toml_file", &detail::ConfigWrapper::LoadTOMLFile,
  //       "Loads the configuration from a `TOML <https://toml.io/en/>`__
  //       file.", pybind11::arg("filename"));

  // // TODO NullValuePolicy
  // m.def(
  //     "load_json_str", &detail::ConfigWrapper::LoadJSONString,
  //     "Loads the configuration from a `JSON <https://www.json.org/>`__
  //     string.", pybind11::arg("json_str"));

  // // TODO NullValuePolicy
  // m.def("load_json_file", &detail::ConfigWrapper::LoadJSONFile,
  //       "Loads the configuration from a `JSON <https://www.json.org/>`__
  //       file.", pybind11::arg("filename"));

  // // TODO libconfig (doc: only available if built with libconfig support)
  // // TODO enable automatically if libconfig++ is installed or enable via
  // //   install extras?
  // m.def("load_libconfig_str", &detail::ConfigWrapper::LoadLibconfigString,
  //       "Loads the configuration from a `Libconfig "
  //       "<http://hyperrealm.github.io/libconfig/>`__ "
  //       "string if `libconfig++` is available.",
  //       pybind11::arg("cfg_str"));

  // m.def("load_libconfig_file", &detail::ConfigWrapper::LoadLibconfigFile,
  //       "Loads the configuration from a `Libconfig "
  //       "<http://hyperrealm.github.io/libconfig/>`__ file.",
  //       pybind11::arg("filename"));

  // //---------------------------------------------------------------------------
  // // Serializing
  // cfg.def("to_toml", &detail::ConfigWrapper::ToTOMLString,
  //         "Returns a `TOML <https://toml.io/>`__-formatted representation "
  //         "of this configuration.");

  // cfg.def("to_json", &detail::ConfigWrapper::ToJSONString,
  //         "Returns a `JSON <https://www.json.org/>`__-formatted
  //         representation " "of this configuration.\n\nNote that date/time
  //         parameters will be " "replaced by their string representation.");

  // cfg.def("to_libconfig", &detail::ConfigWrapper::ToLibconfigString,
  //         "Returns a `Libconfig "
  //         "<http://hyperrealm.github.io/libconfig/>`__-formatted "
  //         "representation of this configuration.");

  // cfg.def("to_yaml", &detail::ConfigWrapper::ToYAMLString,
  //         "Returns a `YAML <https://yaml.org/>`__-formatted representation "
  //         "of this configuration.");
  // // TODO state currently supported version + auto-replacements (date?)

  // cfg.def("to_dict", &detail::ConfigWrapper::ToDict,
  //         "Returns a :class:`dict` holding all parameters of this
  //         configuration.");

  // //---------------------------------------------------------------------------
  // // Getter/Setter

  // detail::RegisterScalarAccess(cfg);
  // detail::RegisterGenericAccess(cfg);

  // //---------------------------------------------------------------------------
  // // Special utils
  // detail::RegisterConfigUtilities(cfg);

  //---------------------------------------------------------------------------
  // Register exceptions
  // The corresponding python module __init__ will override the __module__
  // string of these exceptions. Otherwise, if raised they might confuse the
  // user (due to the binding-internal module name "_core._cfg")

  pybind11::register_local_exception<werkzeugkiste::config::KeyError>(
      m, "KeyError", PyExc_KeyError);
  pybind11::register_local_exception<werkzeugkiste::config::TypeError>(
      m, "TypeError", PyExc_TypeError);
  pybind11::register_local_exception<werkzeugkiste::config::ValueError>(
      m, "ValueError", PyExc_ValueError);
  pybind11::register_local_exception<werkzeugkiste::config::ParseError>(
      m, "ParseError", PyExc_RuntimeError);

  m.attr("KeyError").attr("__doc__") =
      "Raised if an invalid key was provided to access parameters.";
  m.attr("TypeError").attr("__doc__") =
      "Raised if an invalid type was used to get/set a parameter.";
  m.attr("ParseError").attr("__doc__") =
      "Raised if parsing a configuration string/file failed.";
  m.attr("ValueError").attr("__doc__") =
      "Raised if invalid input values have been provided.";

  // // // TODO remove
  // m.def("dev", [m]() {
  //   // auto pydatetime = pybind11::module::import("datetime");
  //   // pybind11::object pydate = pydatetime.attr("date")(2023, 3, 1);

  //   // pybind11::object pytime = pydatetime.attr("time")(23, 49, 10, 123456);

  //   // return pybind11::make_tuple(pydate, pytime);
  //   // // import pyzeugkiste
  //   // // pyzeugkiste._core._cfg.dev()
  //   return pybind11::make_tuple(std::make_optional(42), std::nullopt);
  // });
}
}  // namespace werkzeugkiste::bindings

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_H
