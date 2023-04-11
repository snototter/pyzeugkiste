#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_ACCESS_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_ACCESS_H

#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include <werkzeugkiste/config/casts.h>
#include <werkzeugkiste/config/configuration.h>
#include <werkzeugkiste/strings/strings.h>
#include <werkzeugkiste/logging.h>

#include <memory>
#include <sstream>
#include <stdexcept>
#include <string_view>
#include <utility>
#include <vector>

namespace werkzeugkiste::bindings::detail {

inline void RegisterEnums(pybind11::module &m) {
  //---------------------------------------------------------------------------
  // ConfigType enumeration
  pybind11::enum_<werkzeugkiste::config::ConfigType>(
      m, "ConfigType", "Supported parameter types.")
      .value("Boolean",
          werkzeugkiste::config::ConfigType::Boolean,
          "A flag, *i.e.* :class:`bool`.")
      .value("Integer",
          werkzeugkiste::config::ConfigType::Integer,
          "A signed 64-bit integer, *i.e.* :class:`int`.")
      .value("FloatingPoint",
          werkzeugkiste::config::ConfigType::FloatingPoint,
          "A double-precision floating point number, *i.e.* :class:`float`.")
      .value("String",
          werkzeugkiste::config::ConfigType::String,
          "A string, *i.e.* :class:`str`.")
      .value("Date",
          werkzeugkiste::config::ConfigType::Date,
          "A local date, *i.e.* :class:`datetime.date`.")
      .value("Time",
          werkzeugkiste::config::ConfigType::Time,
          "A local time, *i.e.* :class:`datetime.time`.")
      .value("DateTime",
          werkzeugkiste::config::ConfigType::DateTime,
          "A date-time following `RFC 3339 "
          "<https://www.rfc-editor.org/rfc/rfc3339>`_, *i.e.* "
          ":class:`datetime.datetime`.")
      .value("List",
          werkzeugkiste::config::ConfigType::List,
          "A list of unnamed parameters, convertible to :class:`list`.")
      .value("Group",
          werkzeugkiste::config::ConfigType::Group,
          "A group (collection of named key/value pairs), convertible to "
          ":class:`dict`.");

  //---------------------------------------------------------------------------
  // JSON NullValuePolicy enumeration
  pybind11::enum_<werkzeugkiste::config::NullValuePolicy>(m,
      "NullValuePolicy",
      "How to handle the non-representable ``null`` / ``None`` values.")
      .value("Skip",
          werkzeugkiste::config::NullValuePolicy::Skip,
          "Null values will be skipped, *i.e.* not loaded into the "
          "configuration.")
      .value("NullString",
          werkzeugkiste::config::NullValuePolicy::NullString,
          "Null values will be **replaced** by the string ``'null'``.")
      .value("EmptyList",
          werkzeugkiste::config::NullValuePolicy::EmptyList,
          "Null values will be **replaced** by an empty list.")
      .value("Fail",
          werkzeugkiste::config::NullValuePolicy::Fail,
          "A :class:`~pyzeugkiste.config.ParseError` will be raised upon "
          "parsing null values.");
}

inline void RegisterLoading(pybind11::module &m) {
  //---------------------------------------------------------------------------
  // Loading a configuration

  std::string doc_string = R"doc(
      Loads a configuration file.

      The configuration type will be deduced from the file extension, *i.e.*
      ``.toml``, ``.json``, or ``.cfg``.
      For JSON files, the default :class:`~pyzeugkiste.config.NullValuePolicy`
      will be used, see :meth:`load_json_file`.

      Args:
        filename: Path to the configuration file. Can either be a :class:`str` or
          any object that can be represented as a :class:`str`. For example, a
          :class:`pathlib.Path` is also a valid input parameter.

      Raises:
        :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured, *e.g.* the
          file does not exist, the configuration type cannot be deduced, there are
          syntax errors in the file, *etc.*
      )doc";
  m.def(
      "load", &Config::LoadFile, doc_string.c_str(), pybind11::arg("filename"));

  doc_string = R"doc(
      Loads the configuration from a `TOML <https://toml.io/en/>`__ string.

      Args:
        toml_str: Configuration as :class:`str`.

      Raises:
        :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured.
      )doc";
  m.def("load_toml_str",
      &Config::LoadTOMLString,
      doc_string.c_str(),
      pybind11::arg("toml_str"));

  doc_string = R"doc(
      Loads the configuration from a `TOML <https://toml.io/en/>`__ file.

      Args:
        filename: Path to the configuration file. Can either be a :class:`str` or
          any object that can be represented as a :class:`str`. For example, a
          :class:`pathlib.Path` is also a valid input parameter.

      Raises:
        :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured,
          *e.g.* if the file does not exist, there were syntax errors, *etc.*
      )doc";
  m.def("load_toml_file",
      &Config::LoadTOMLFile,
      doc_string.c_str(),
      pybind11::arg("filename"));

  doc_string = R"doc(
      Loads the configuration from a `JSON <https://www.json.org/>`__ string.

      Args:
        json_str: Configuration as :class:`str`.
        none_policy: A :class:`~pyzeugkiste.config.NullValuePolicy` enum which
          specifies how ``None`` or ``null`` values should be handled.

      Raises:
        :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured.
      )doc";
  m.def("load_json_str",
      &Config::LoadJSONString,
      doc_string.c_str(),
      pybind11::arg("json_str"),
      pybind11::arg("none_policy") =
          werkzeugkiste::config::NullValuePolicy::Skip);

  doc_string = R"doc(
      Loads the configuration from a `JSON <https://www.json.org/>`__ file.

      Args:
        filename: Path to the configuration file. Can either be a :class:`str` or
          any object that can be represented as a :class:`str`. For example, a
          :class:`pathlib.Path` is also a valid input parameter.
        none_policy: A :class:`~pyzeugkiste.config.NullValuePolicy` enum which
          specifies how ``None`` or ``null`` values should be handled.

      Raises:
        :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured,
          *e.g.* if the file does not exist, there were syntax errors, *etc.*
      )doc";
  m.def("load_json_file",
      &Config::LoadJSONFile,
      doc_string.c_str(),
      pybind11::arg("filename"),
      pybind11::arg("none_policy") =
          werkzeugkiste::config::NullValuePolicy::Skip);

  doc_string = R"doc(
      Loads the configuration from a `Libconfig <http://hyperrealm.github.io/libconfig/>`__ string.

      This functionality requires ``libconfig++``. If CMake can locate the
      library, ``pyzeugkiste`` will be built with libconfig support.

      Args:
        cfg_str: Configuration as :class:`str`.

      Raises:
        :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured.
        :class:`RuntimeError`: If libconfig support is not available.
      )doc";
  m.def("load_libconfig_str",
      &Config::LoadLibconfigString,
      doc_string.c_str(),
      pybind11::arg("cfg_str"));

  doc_string = R"doc(
      Loads the configuration from a `Libconfig <http://hyperrealm.github.io/libconfig/>`__ file.

      This functionality requires ``libconfig++``. If CMake can locate the
      library, ``pyzeugkiste`` will be built with libconfig support.

      Args:
        filename: Path to the configuration file. Can either be a :class:`str` or
          any object that can be represented as a :class:`str`. For example, a
          :class:`pathlib.Path` is also a valid input parameter.

      Raises:
        :class:`~pyzeugkiste.config.ParseError`: If a parsing error occured,
          *e.g.* if the file does not exist, there were syntax errors, *etc.*
        :class:`RuntimeError`: If libconfig support is not available.
      )doc";
  m.def("load_libconfig_file",
      &Config::LoadLibconfigFile,
      doc_string.c_str(),
      pybind11::arg("filename"));
}

inline void RegisterBasicOperators(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // General utils/operators
  std::string doc_string = R"doc(
      Checks for equality.

      Args:
        other: Either another :class:`~pyzeugkiste.config.Config` instance or
          a :class:`dict`. The latter will be implicitly converted to
          :class:`~pyzeugkiste.config.Config` before comparison.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``other`` is neither a
          :class:`~pyzeugkiste.config.Config` instance nor a :class:`dict`.

      Returns:
        ``True`` if both configs contain the exact same configuration,
        *i.e.* keys, corresponding data types and values.

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
      )doc";
  wrapper.def(
      "__eq__",
      [](const Config &self, pybind11::handle other) -> bool {
        return self.Equals(other);
      },
      doc_string.c_str(),
      pybind11::arg("other"));

  wrapper.def(
      "__ne__",
      [](const Config &self, pybind11::handle other) -> bool {
        return !self.Equals(other);
      },
      "Checks for inequality, see :meth:`__eq__` for details.",
      pybind11::arg("other"));

  wrapper.def("copy", &Config::Copy, "Returns a deeply copied configuration.");

  wrapper.def("__contains__",
      &Config::Contains,
      "Checks if the given key/parameter name exists.",
      pybind11::arg("key"));

  doc_string = R"doc(
      Returns a read-only iterator for this group or list of parameters.

      If the :class:`~pyzeugkiste.config.Config` instance is a group of
      parameters, ``__iter___`` iterates over the parameter names.
      If it is a view of a parameter list, ``__iter__`` iterates over
      the corresponding values. Note that the values are returned as
      copies and nested lists & groups will be converted to plain
      python :class:`list` and :class:`dict`, respectively.
      )doc";
  wrapper.def(
      "__iter__",
      [](const Config &self) {
        return pybind11::make_iterator(self.cbegin(), self.cend());
      },
      "Iterate over parameter names (for groups) or items (for views on "
      "lists).",
      // The Config must be kept alive while iterator exists:
      pybind11::keep_alive<0, 1>());

  wrapper.def("parameter_len",
      &Config::ParameterLength,
      "Returns the number of child parameters for the given key.",
      pybind11::arg("key"));

  wrapper.def("__len__",
      &Config::Length,
      "Returns the number of parameters, *i.e.* direct children of "
      "this configuration.");

  wrapper.def("empty",
      &Config::Empty,
      "Checks if this configuration contains any parameters.");

  // TODO raises
  doc_string = R"doc(
      Returns the parameter's :class:`~pyzeugkiste.config.ConfigType`.

      Args:
        key: The fully qualified parameter name. If an empty :class:`str` is
          provided, the type of the current configuration (either
          :class:`~pyzeugkiste.config.ConfigType.Group` or
          :class:`~pyzeugkiste.config.ConfigType.List`) will be
          returned.
      )doc";
  wrapper.def("type",
      &Config::ParameterType,
      doc_string.c_str(),
      pybind11::arg("key") = std::string{});

  // TODO raises
  doc_string = R"doc(
      Returns the parameter names/keys of the direct child nodes (first-level
      parameters) of this configuration.

      Note that the returned :class:`list` of parameter names is a snapshot/copy,
      **not** a dynamic view. If the configuration changes, any previously
      returned list of parameter names will not be updated automatically.

      To retrieve parameter names of a selected sub-group, or to recursively
      list all parameters, :meth:`list_parameter_names` should be used instead.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::ListParameterNames``.

      .. code-block:: python
         :caption: Example: Key queries

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

  // TODO raises typeerror
  doc_string = R"doc(
      Returns (copies of) the parameter values.

      Note that the returned :class:`list` of parameter values is a copy,
      **not a dynamic view**. If the configuration changes, any previously
      returned list of parameter values will not be updated automatically.

      Changing a value **will not change** the corresponding parameter of
      the :class:`Config` object.

      Aggregate parameters (lists and groups) will be converted to their
      corresponding python type, *i.e.* :class:`list` and :class:`dict`.
      )doc";
  wrapper.def("values", &Config::Values, doc_string.c_str());

  // TODO raises typeerror
  doc_string = R"doc(
      Returns a copy of the parameter group's items, *i.e.* (key, value) pairs.

      Note that the returned :class:`list` of (key, value) pairs values is
      **not a dynamic view**, but a copy. If the configuration changes, any
      previously returned list of items will not be updated automatically.

      Changing a value **will not change** the corresponding parameter of
      the :class:`Config` object.

      Aggregate parameters (lists and groups) will be converted to their
      corresponding python type, *i.e.* :class:`list` and :class:`dict`.
      )doc";
  wrapper.def("items", &Config::Items, doc_string.c_str());

  doc_string = R"doc(
      Removes all parameters of this configuration.

      If this instance provides a view on a list parameter, this call will
      delete all list elements, resulting in an empty list.
      If this instance represents a (sub-)group, this call will delete all
      child parameters, resulting in an empty (sub-)group.

      .. code-block:: python
         :caption: Example: Clearing lists and configuration groups

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
              str = 'value'

              [group]
              int = 42
              lst = [1, 2, 3]
              """)

         cfg['group']['lst'].clear()
         assert cfg['group']['lst'].empty()

         cfg['group'].clear()
         assert cfg['group'].empty()

         cfg.clear()
         assert cfg.empty()
      )doc";
  wrapper.def("clear", &Config::Clear, doc_string.c_str());
}

inline void RegisterSerialization(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // Serialization
  wrapper.def("to_toml",
      &Config::ToTOMLString,
      "Returns a `TOML <https://toml.io/>`__-formatted representation "
      "of this configuration.");

  wrapper.def("to_json",
      &Config::ToJSONString,
      "Returns a `JSON <https://www.json.org/>`__-formatted representation "
      "of this configuration.\n\nNote that date/time parameters will be "
      "replaced by their string representation.");

  wrapper.def("to_libconfig",
      &Config::ToLibconfigString,
      "Returns a `Libconfig "
      "<http://hyperrealm.github.io/libconfig/>`__-formatted "
      "representation of this configuration.\n\nNote that date/time "
      "parameters will be replaced by their string representation.");

  wrapper.def("to_yaml",
      &Config::ToYAMLString,
      "Returns a `YAML <https://yaml.org/>`__-formatted representation "
      "of this configuration.\n\nNote that date/time parameters will be "
      "replaced by their string representation.");

  wrapper.def("to_dict",
      &Config::ToDict,
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
      .def("__repr__", [wrapper](const Config &c) {
        std::ostringstream s;
        const std::string modname =
            wrapper.attr("__module__").cast<std::string>();
        if (!modname.empty()) {
          s << modname << '.';
        }
        const auto lines = werkzeugkiste::strings::Split(
            werkzeugkiste::strings::Replace(c.ToTOMLString(), "\"\"\"", "'''"),
            '\n');
        // TODO the replacement could lead to invalid TOML
        s << "load_toml_str(\"\"\"\n";
        for (const auto &line : lines) {
          s << werkzeugkiste::strings::Indent(line, 2, ' ') << '\n';
        }
        s << "  \"\"\")";
        return s.str();
      });
}

inline void RegisterGenericAccess(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // Generic access
  std::string doc_string = R"doc(
      Looks up an element.

      A :class:`~pyzeugkiste.config.Config` instance can either *view/refer*
      to a *(sub-)group* of **named** parameters or a *list* of **unnamed**
      parameters.
      If it refers to a *(sub-)group*, the lookup ``arg`` must be a :class:`str`
      holding the desired key (*i.e.* its parameter name).
      If it refers to a *list*, the lookup ``arg`` must be an index, specified
      as :class:`int`. For lists, negative lookup indices are supported.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If the parameter at the
          given lookup ``arg`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``arg`` is neither a
          :class:`str` or :class:`int`.

      Args:
        arg: Indexing object, either :class:`str` or :class:`int`. See
          above documentation for details.

      .. code-block:: python
         :caption: Example: parameter lookup

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
             int = 3
             lst = [1, 2, 3]

             [group]
             int = 17
             """)

         cfg['int']  # Returns the scalar as built-in type, i.e. int(3)

         cfg['lst']  # Returns a "list view" of type pyc.Config
         cfg['lst'][0]  # Returns the first list element, i.e. int(1)
         cfg['lst[0]']  # Will also return the first element

         cfg['group']  # Returns a "sub-group view" of type pyc.Config
         cfg['group']['int']  # Returns int(17)
         cfg['group.int']     # Will also return int(17)

         # To create a deep copy instead of working with a "view", use 'copy':
         another_cfg = cfg['group'].copy()

         cfg[0]             # Raises a pyc.KeyError
         cfg['no-such-key'] # Raises a pyc.KeyError
         cfg['lst'][17]     # Raises a pyc.KeyError
         cfg[None]  # Raises a pyc.TypeError
      )doc";
  wrapper.def(
      "__getitem__",
      [](Config &self, pybind11::handle key) -> pybind11::object {
        if (pybind11::isinstance<pybind11::int_>(key)) {
          return self.GetScalarOrView(key.cast<int>());
        }

        if (pybind11::isinstance<pybind11::str>(key)) {
          return self.GetScalarOrView(key.cast<std::string>());
        }

        const std::string py_typestr =
            pybind11::cast<std::string>(key.attr("__class__").attr("__name__"));
        std::string msg{
            "Invalid key argument in Config.__getitem__: type must be str or "
            "int, but got `"};
        msg += py_typestr;
        msg += "`!";
        throw werkzeugkiste::config::TypeError{msg};
      },
      doc_string.c_str(),
      pybind11::arg("arg"));

  doc_string = R"doc(
      Creates or updates a parameter.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If the parameter exists, but
          is of a different type (*i.e.* changing the type of a parameter is not
          supported), or if the parent path could not be created (*e.g.* if
          creating this parameter would require to implicitly create
          a parent list). This error will also be raised if the lookup ``arg``
          is neither a :class:`str` (*i.e.* a parameter name) or an
          :class:`int` (*i.e.* an index into a list).

      Args:
        arg: Indexing object, either :class:`str` or :class:`int`. See
          documentation of :meth:`__getitem__` for details.

      .. code-block:: python
         :caption: Example: Setting parameters

         from pyzeugkiste import config as pyc
         cfg = pyc.Config()

         # Create parameters
         cfg['str'] = 'value'
         cfg['answer'] = 42
         cfg['lst'] = [1, 2, [3, 4]]

         # When using a dictionary, all its keys must be of type str:
         cfg['group'] = {'name': 'test', 'value': 3}

         # Updating parameters
         cfg['answer'] = 0
         cfg['lst'] = (1, 2)  # A list can also be set from a tuple
         cfg['group']['name'] = 'foo'

         # Numeric parameters can be set using "compatible" types, i.e. any
         # number that can be exactly represented by the parameter's data type:
         cfg['answer'] = 3.0  # float(3.0) can be converted to int(3)
         cfg['answer'] = 3.5  # Raises a pyc.TypeError

         # Changing the type of an existing parameter is not allowed:
         cfg['str'] = True  # Raises a pyc.TypeError
      )doc";
  wrapper.def(
      "__setitem__",
      [](Config &self, pybind11::handle key, pybind11::handle value) -> void {
        if (pybind11::isinstance<pybind11::int_>(key)) {
          self.SetIndex(key.cast<int>(), value);
        } else if (pybind11::isinstance<pybind11::str>(key)) {
          self.SetKey(key.cast<std::string>(), value);
        } else {
          const std::string py_typestr = pybind11::cast<std::string>(
              key.attr("__class__").attr("__name__"));
          std::string msg{
              "Invalid key argument in Config.__setitem__: type must be str or "
              "int, but got `"};
          msg += py_typestr;
          msg += "`!";
          throw werkzeugkiste::config::TypeError{msg};
        }
      },
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Appends a value to a list parameter.

      Args:
        value: The object/parameter value to be appended.
        key: The fully qualified parameter name of the list. If an empty
          :class:`str` is provided, this configuration object must be
          a view of a list parameter.
          If the key does not exist, it will be created.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If the parameter exists, but
          is not a list.

      .. code-block:: python
         :caption: Example: Appending values to a list

         from pyzeugkiste import config as pyc
         cfg = pyc.Config()
         cfg['lst'] = [1, 2, 3]

         cfg['lst'].append(4)
         # cfg['lst'] is now: [1, 2, 3, 4]

         cfg.append('five', key='lst')
         # cfg['lst'] is now: [1, 2, 3, 4, 'five']

         # 'append' can also be used to create a new list parameter:
         cfg.append(42, key='the-answer')
         # cfg['the-answer'] is now: [42]
      )doc";
  wrapper.def(
      "append",
      [](Config &self, pybind11::handle value, std::string_view key) -> void {
        self.Append(key, value);
      },
      doc_string.c_str(),
      pybind11::arg("value"),
      pybind11::arg("key") = std::string_view{});

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
         del cfg['arr'][0]  # Raises a TypeError (due to the python/c++ bindings)

      )doc";
  wrapper.def(
      "__delitem__",
      [](Config &self, std::string_view key) -> void { self.Delete(key); },
      doc_string.c_str(),
      pybind11::arg("key"));
}

inline void RegisterTypedAccess(pybind11::class_<Config> &wrapper) {
  //---------------------------------------------------------------------------
  // Typed queries
  std::string doc_string = R"doc(
      Returns the :class:`bool` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetBoolean``.

      Args:
        key: The fully qualified parameter name.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a boolean parameter.
      )doc";
  wrapper.def(
      "bool", &Config::GetBool, doc_string.c_str(), pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`bool` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetBooleanOr``.

      Args:
        key: The fully qualified parameter name.
        value: Any object to be returned if the ``key`` does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a boolean parameter.

      Returns:
        The parameter ``value`` if the ``key`` does not exist. Otherwise, the
        actual parameter value will be returned.
      )doc";
  wrapper.def("bool_or",
      &Config::GetBoolOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`int` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetInt64``.

      Args:
        key: The fully qualified parameter name.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but cannot
          be cast to (*i.e.* exactly represented by) an :class:`int`.
      )doc";
  wrapper.def("int", &Config::GetInt, doc_string.c_str(), pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`int` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetInt64Or``.

      Args:
        key: The fully qualified parameter name.
        value: Any object to be returned if the ``key`` does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but cannot
          be cast to (*i.e.* exactly represented by) an :class:`int`.

      Returns:
        The ``value`` object if the ``key`` does not exist. Otherwise, the
        actual parameter value will be returned.
      )doc";
  wrapper.def("int_or",
      &Config::GetIntOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`float` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetDouble``.

      Args:
        key: The fully qualified parameter name.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but cannot
          be cast to (*i.e.* exactly represented by) a :class:`float`.
      )doc";
  wrapper.def(
      "float", &Config::GetFloat, doc_string.c_str(), pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`float` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetDoubleOr``.

      Args:
        key: The fully qualified parameter name.
        value: Any object to be returned if the ``key`` does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but cannot
          be cast to (*i.e.* exactly represented by) a :class:`float`.

      Returns:
        The ``value`` object if the ``key`` does not exist. Otherwise, the
        actual parameter value will be returned.
      )doc";
  wrapper.def("float_or",
      &Config::GetFloatOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`str` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetString``.

      Args:
        key: The fully qualified parameter name.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`str`.
      )doc";
  wrapper.def("str", &Config::GetStr, doc_string.c_str(), pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`str` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetStringOr``.

      Args:
        key: The fully qualified parameter name.
        value: Any object to be returned if the ``key`` does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`str`.

      Returns:
        The ``value`` object if the ``key`` does not exist. Otherwise, the
        actual parameter value will be returned.
      )doc";
  wrapper.def("str_or",
      &Config::GetStrOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`datetime.date` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetDate``.

      Args:
        key: The fully qualified parameter name.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.date`.
      )doc";
  wrapper.def(
      "date", &Config::GetDate, doc_string.c_str(), pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`datetime.date` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetDateOr``.

      Args:
        key: The fully qualified parameter name.
        value: Any object to be returned if the ``key`` does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.date`.

      Returns:
        The ``value`` object if the ``key`` does not exist. Otherwise, the
        actual parameter value will be returned.
      )doc";
  wrapper.def("date_or",
      &Config::GetDateOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`datetime.time` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetTime``.

      Args:
        key: The fully qualified parameter name.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.time`.
      )doc";
  wrapper.def(
      "time", &Config::GetTime, doc_string.c_str(), pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`datetime.time` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetTimeOr``.

      Args:
        key: The fully qualified parameter name.
        value: Any object to be returned if the ``key`` does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.time`.

      Returns:
        The ``value`` object if the ``key`` does not exist. Otherwise, the
        actual parameter value will be returned.
      )doc";
  wrapper.def("time_or",
      &Config::GetTimeOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Returns the :class:`datetime.datetime` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetDateTime``.

      Args:
        key: The fully qualified parameter name.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If ``key`` does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.datetime`.
      )doc";
  wrapper.def("datetime",
      &Config::GetDateTime,
      doc_string.c_str(),
      pybind11::arg("key"));

  doc_string = R"doc(
      Returns an optional :class:`datetime.datetime` parameter.

      **Corresponding C++ API:** ``werkzeugkiste::config::Configuration::GetDateTimeOr``.

      Args:
        key: The fully qualified parameter name.
        value: Any object to be returned if the ``key`` does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If ``key`` exists, but is not
          a :class:`datetime.datetime`.

      Returns:
        The ``value`` object if the ``key`` does not exist. Otherwise, the
        actual parameter value will be returned.
      )doc";
  wrapper.def("datetime_or",
      &Config::GetDateTimeOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Returns a copy of the list parameter as :class:`list`.

      If the list parameter contains any nested :class:`Config`
      instances, they will be converted to :class:`dict`.

      Args:
        key: If a non-empty :class:`str` is provided, it is interpreted as the
          fully qualified parameter name of a list parameter. Otherwise,
          ``self`` must be a view of an existing list parameter.

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
      )doc";
  wrapper.def("list",
      &Config::GetList,
      doc_string.c_str(),
      pybind11::arg("key") = std::string{});

  doc_string = R"doc(
      Returns a copied list or the given value if the parameter does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If the parameter exists, but is
          not a list.

      Args:
        key: The fully qualified parameter name of a list parameter. If an
          empty :class:`str` is provided, ``self`` must be a view of an
          existing list parameter.
        value: Any object to be returned if the given ``key`` does not exist.
      )doc";
  wrapper.def("list_or",
      &Config::GetListOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));

  doc_string = R"doc(
      Returns a copy of the group parameter as :class:`dict`.

      If the group parameter contains any nested :class:`Config`
      instances, they will also be converted to :class:`dict`.

      Args:
        key: If a non-empty :class:`str` is provided, it is interpreted as the
          fully qualified parameter name of a list parameter. Otherwise,
          ``self`` must be a view of an existing list parameter.

      .. code-block:: python
         :caption: Retrieve a deeply copied dict

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
             [group]
             str = 'value'
             lst = [1, 2, 3]
             """)

         cfg.dict()
         # Returns {'group': {'str': 'value', 'lst': [1, 2, 3]}}

         cfg.dict('group')
         cfg['group'].dict()
         # Returns {'str': 'value', 'lst': [1, 2, 3]}
      )doc";
  wrapper.def("dict",
      &Config::GetDict,
      doc_string.c_str(),
      pybind11::arg("key") = std::string{});

  doc_string = R"doc(
      Returns a copied group as :class:`dict` or the given value if the
      parameter does not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If the parameter exists, but is
          not a group, *i.e.* not convertible to a :class:`dict`.

      Args:
        key: fully qualified parameter name of a parameter group. If an
          empty :class:`str` is provided, ``self`` must be a view of an
          existing parameter group.
        value: Any object to be returned if the given ``key`` does not exist.
      )doc";
  wrapper.def("dict_or",
      &Config::GetDictOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("value"));
  
  // TODO add example dtype=np.int32 vs dtype=other_mat.dtype
  // TODO update list of supported types
  doc_string = R"doc(
      Returns a list/nested list parameter as :class:`numpy.ndarray`.

      Raises:
        :class:`~pyzeugkiste.config.KeyError`: If the parameter does not exist.
        :class:`~pyzeugkiste.config.TypeError`: If the parameter exists, but is
          not a list.

      Args:
        key: Fully qualified parameter name.
        dtype: Type of the output :class:`numpy.ndarray`. Can either be a 
          `NumPy type <https://numpy.org/doc/stable/user/basics.types.html>`__ 
          or a :class:`numpy.dtype`.
          The following types are supported: :class:`numpy.float64`,
          :class:`numpy.float32`, :class:`numpy.int64`, :class:`numpy.int32`,
          and :class:`numpy.uint8`.
      
      .. code-block:: python
         :caption: Example: Query list parameters as numpy.ndarray

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
            mat = [
              [800,   0, 200],
              [  0, 750, 255]
            
            lst = [1, 2, 3]
            ]""")

         mat = cfg['mat'].numpy(dtype=np.int32)
         mat = cfg.numpy('mat', dtype=np.int32)
         assert mat.dtype == np.int32
         assert mat.shape == (2, 3)

         # The following will raise a pyc.TypeError, because the values
         # cannot be represented by the given dtype (max value for uint8
         # is 255).
         cfg['mat'].numpy(dtype=np.uint8)

         # A single list ("1D matrix") will always be returned as a
         # 2D matrix with shape (N, 1), i.e. a row vector.          
         vec = cfg['lst'].numpy(dtype=np.float32)
         vec = cfg.numpy('lst', dtype=np.float32)
         assert vec.dtype == np.float32
         assert vec.ndim == 2
         assert vec.shape == (3, 1)
      )doc";
  wrapper.def("numpy",
      &Config::GetMatrix,
      doc_string.c_str(),
      pybind11::arg("key") = std::string{},
      pybind11::arg("dtype") = pybind11::dtype("float64"));

   doc_string = R"doc(
      Returns a :class:`numpy.ndarray` or the given value if the parameter does
      not exist.

      Raises:
        :class:`~pyzeugkiste.config.TypeError`: If the parameter exists, but is
          not a list.

      Args:
        key: fully qualified parameter name.
        dtype: Type of the output :class:`numpy.ndarray`. Can either be a 
          `NumPy type <https://numpy.org/doc/stable/user/basics.types.html>`__ 
          or a :class:`numpy.dtype`.
          The following types are supported: :class:`numpy.float64`,
          :class:`numpy.float32`, :class:`numpy.int64`, :class:`numpy.int32`,
          and :class:`numpy.uint8`.
        value: Any object to be returned if the given ``key`` does not exist. 
      )doc";
  wrapper.def("numpy_or",
      &Config::GetMatrixOr,
      doc_string.c_str(),
      pybind11::arg("key"),
      pybind11::arg("dtype"),
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
        :class:`~pyzeugkiste.config.ParseError`: Upon parsing errors, such as
          file not found, invalid syntax, *etc.*
        :class:`~pyzeugkiste.config.TypeError`: If the given parameter is
          not a string.
        :class:`RuntimeError`: If the configuration could not be inserted. In
          such cases, please file a bug report.

      Args:
        key: The fully qualified parameter name which holds the file name
          of the nested `TOML <https://toml.io/en/>`__ configuration (must
          be of type string).
      )doc";
  wrapper.def("load_nested",
      &Config::LoadNested,
      doc_string.c_str(),
      pybind11::arg("key"));

  doc_string = R"doc(
      Adjusts string parameters which hold relative file paths.

      After invocation, the given parameters hold either an absolute file path,
      or the concatenation result ``"base_path / <param>"`` if they initially
      held a relative file path.

      To check and adjust such paths, either the fully qualified names
      of all parameters can be provided, such as
      ``['file_path', 'storage.image_path', 'storage.doc_path', ...]``, or a
      pattern which uses the wildcard ``'*'``.
      For example, to adjust **all** parameters which names end with
      the suffix ``_path`` as above, we could simply pass ``['*_path']``.

      If a non-string parameter would match a given name/pattern, it will
      be ignored.

      Args:
        base_path: Base path to be prepended to relative file paths. Can either
          be a :class:`str` or a :class:`pathlib.Path`.
        parameters: A list of parameter names or patterns.
        key: If a non-empty :class:`str` is provided, it is interpreted as the
          fully qualified parameter name of a sub-group. Only matching
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
  wrapper.def("adjust_relative_paths",
      &Config::AdjustRelativePaths,
      doc_string.c_str(),
      pybind11::arg("base_path"),
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
          fully qualified parameter name of a sub-group. Replacements will only
          affect parameters contained in this sub-group.

      Raises:
        :class:`RuntimeError`: If a provided *search_str* is empty.
        :class:`~pyzeugkiste.config.KeyError`: If a sub-group ``key`` was
          specified but does not exist.

      Returns:
        ``True`` if any placeholder has actually been replaced.

      .. code-block:: python
         :caption: Replace placeholders

         from pyzeugkiste import config as pyc
         cfg = pyc.load_toml_str("""
             str = 'Release: %VERSION%'
             token = '%TOKEN% - %TOKEN%'
             """)

         cfg.replace_placeholders([
             ('%VERSION%', 'v0.1'), ('%TOKEN%', '1337')])

         # Resulting configuration:
         # str = 'Release: v0.1'
         # token = '1337 - 1337'
      )doc";
  wrapper.def("replace_placeholders",
      &Config::ReplacePlaceholders,
      doc_string.c_str(),
      pybind11::arg("placeholders"),
      pybind11::arg("key") = std::string{});

  doc_string = R"doc(
      Returns the fully qualified names/keys of all parameters below
      the root or specified sub-group.

      **Corresponding C++ API:**
      ``werkzeugkiste::config::Configuration::ListParameterNames``.

      Args:
        include_array_entries: If ``True``, the name of each parameter will
          be returned, *i.e.* the result will also include **each** array
          element. Otherwise, only explicitly named parameters will be
          included, refer to the example below.
        recursive: If ``True``, parameter names will be listed recursively.
          Otherwise, only the direct *children* will be listed.
        key: If a non-empty :class:`str` is provided, it is interpreted as the
          fully qualified parameter name of a sub-group. Replacements will only
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


         param_names = cfg.list_parameter_names(recursive = False)
         # Returns [
         #   'str1',
         #   'values'
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
  wrapper.def("list_parameter_names",
      &Config::ListParameterNames,
      doc_string.c_str(),
      pybind11::arg("include_array_entries") = false,
      pybind11::arg("recursive") = true,
      pybind11::arg("key") = std::string{});
}
}  // namespace werkzeugkiste::bindings::detail

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_DETAIL_ACCESS_H
