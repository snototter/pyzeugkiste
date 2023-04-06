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

namespace werkzeugkiste::bindings::detail {
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
    std::string_view key,
    pybind11::handle lst);
}  // namespace werkzeugkiste::bindings::detail

#include <werkzeugkiste-bindings/detail/config_bindings_types.h>
#include <werkzeugkiste-bindings/detail/config_bindings_access.h>

namespace werkzeugkiste::bindings {
inline void RegisterConfigUtils(pybind11::module &main_module) {
  pybind11::module m = main_module.def_submodule("_cfg");
  m.doc() = R"doc(
    Configuration file utils.

    Allows a unified handling of different configuration formats, *i.e.*
    `TOML <https://toml.io/en/>`__, `JSON <https://www.json.org/>`__ and
    `libconfig <http://hyperrealm.github.io/libconfig/>`__.
    )doc";

  detail::RegisterConfigTypes(m);

  std::string doc_string = R"doc(
    Encapsulates parameters.

    This class provides dictionary-like access to parameters and
    provides several additional utilities, such as replacing placeholders,
    adjusting relative file paths, *etc.*

    This utitility class is intended for *"typical"*, human-friendly
    configuration scenarios and, similar to `TOML <https://toml.io/en/>`__,
    supports the following data types:

      * Basic scalars: :class:`bool`, :class:`int`, :class:`float`, and
        :class:`str`.
      * Date time types: :class:`~datetime.date`, :class:`~datetime.time`,
        and :class:`~datetime.datetime`.
      * Aggregate types, *i.e.* :class:`list` and collections/groups
        (python equivalent: :class:`dict`).

    The following configuration formats are supported:

      * `TOML <https://toml.io/en/>`__
      * `JSON <https://www.json.org/>`__
      * `libconfig <http://hyperrealm.github.io/libconfig/>`__
      * `YAML <https://yaml.org/>`__ (only for exporting)

    **Default access** is supported via the indexing operator ``[]``,
    *i.e.* :meth:`__getitem__` and :meth:`__setitem__`.

    **Explicitly type-checked access** is supported via :meth:`int`,
    :meth:`date`, *etc.*
    To return default values if a *key* does not exist, :meth:`int_or`,
    :meth:`float_or`, *etc.* can be used.

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

  pybind11::class_<detail::Config> wrapper(m, "Config", doc_string.c_str());
  wrapper.def(pybind11::init<>(), "Creates an empty configuration.");

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
}
}  // namespace werkzeugkiste::bindings

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_H
