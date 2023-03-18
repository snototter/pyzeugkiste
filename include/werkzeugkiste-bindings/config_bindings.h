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

namespace wzkcfg = werkzeugkiste::config;

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
print(c.to_toml_str())


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

namespace werkzeugkiste::bindings {
namespace detail {
inline wzkcfg::date PyObjToDateUnchecked(pybind11::handle obj) {
  const int year = obj.attr("year").cast<int>();
  const int month = obj.attr("month").cast<int>();
  const int day = obj.attr("day").cast<int>();
  return wzkcfg::date{static_cast<uint_fast16_t>(year),
                      static_cast<uint_fast8_t>(month),
                      static_cast<uint_fast8_t>(day)};
}

inline wzkcfg::time PyObjToTimeUnchecked(pybind11::handle obj) {
  const int hour = obj.attr("hour").cast<int>();
  const int minute = obj.attr("minute").cast<int>();
  const int second = obj.attr("second").cast<int>();
  const int microsec = obj.attr("microsecond").cast<int>();
  return wzkcfg::time{static_cast<uint_fast8_t>(hour),
                      static_cast<uint_fast8_t>(minute),
                      static_cast<uint_fast8_t>(second),
                      static_cast<uint_fast32_t>(microsec * 1000)};
}

inline wzkcfg::date PyObjToDate(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return wzkcfg::date{obj.cast<std::string>()};
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
  throw wzkcfg::TypeError(msg);
}

inline wzkcfg::time PyObjToTime(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return wzkcfg::time{obj.cast<std::string>()};
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
  throw wzkcfg::TypeError(msg);
}

inline wzkcfg::date_time PyObjToDateTime(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return wzkcfg::date_time{obj.cast<std::string>()};
  }

  // We need to ensure that the PyDateTime import is initialized.
  // Or prepare for segfaults.
  if (!PyDateTimeAPI) {
    PyDateTime_IMPORT;
  }

  if (PyDateTime_Check(obj.ptr())) {
    // Object is datetime.datetime or subtype
    const wzkcfg::date d = PyObjToDateUnchecked(obj);
    const wzkcfg::time t = PyObjToTimeUnchecked(obj);

    if (obj.attr("tzinfo").is_none()) {
      return wzkcfg::date_time{d, t};
    } else {
      const auto pyoffset_sec = obj.attr("utcoffset")().attr("total_seconds")();
      const auto offset_min =
          static_cast<int_fast16_t>(pyoffset_sec.cast<double>() / 60.0);
      return wzkcfg::date_time{d, t, wzkcfg::time_offset{offset_min}};
    }
  }

  const std::string tp =
      pybind11::cast<std::string>(obj.attr("__class__").attr("__name__"));
  std::string msg{"Cannot convert python type `"};
  msg += tp;
  msg += "` to `werkzeugkiste::date_time`!";
  throw wzkcfg::TypeError(msg);
}

inline pybind11::object DateToPyObj(const wzkcfg::date &d) {
  auto pydatetime = pybind11::module::import("datetime");
  return pydatetime.attr("date")(d.year, d.month, d.day);
}

inline pybind11::object TimeToPyObj(const wzkcfg::time &t) {
  auto pydatetime = pybind11::module::import("datetime");
  return pydatetime.attr("time")(t.hour, t.minute, t.second,
                                 t.nanosecond / 1000);
}

inline pybind11::object DateTimeToPyObj(const wzkcfg::date_time &dt) {
  auto pydatetime = pybind11::module::import("datetime");
  if (dt.IsLocal()) {
    return pydatetime.attr("datetime")(
        dt.date.year, dt.date.month, dt.date.day, dt.time.hour, dt.time.minute,
        dt.time.second, dt.time.nanosecond / 1000);
  }

  auto timedelta = pydatetime.attr("timedelta");
  auto timezone = pydatetime.attr("timezone");
  // Ctor parameter order is: (days=0, seconds=0, microseconds=0,
  //   milliseconds=0, minutes=0, hours=0, weeks=0)
  // https://docs.python.org/3/library/datetime.html#datetime.timedelta
  auto offset_td = timedelta(0, 0, 0, 0, dt.offset.value().minutes);
  return pydatetime.attr("datetime")(
      dt.date.year, dt.date.month, dt.date.day, dt.time.hour, dt.time.minute,
      dt.time.second, dt.time.nanosecond / 1000, timezone(offset_td));
}

inline std::string PathStringFromPython(pybind11::handle path) {
  if (pybind11::isinstance<pybind11::str>(path)) {
    return path.cast<std::string>();
  } else {
    return pybind11::str(path).cast<std::string>();
  }
}

class ConfigWrapper {
 public:
  static ConfigWrapper LoadFile(pybind11::handle filename) {
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::LoadFile(PathStringFromPython(filename));
    return instance;
  }

  static ConfigWrapper LoadTOMLFile(pybind11::handle filename) {
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::LoadTOMLFile(PathStringFromPython(filename));
    return instance;
  }

  static ConfigWrapper LoadTOMLString(std::string_view toml_str) {
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::LoadTOMLString(toml_str);
    return instance;
  }

  static ConfigWrapper LoadJSONFile(pybind11::handle filename) {
    // TODO support NullValuePolicy
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::LoadJSONFile(PathStringFromPython(filename));
    return instance;
  }

  static ConfigWrapper LoadJSONString(std::string_view json_str) {
    // TODO support NullValuePolicy
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::LoadJSONString(json_str);
    return instance;
  }

  static ConfigWrapper LoadLibconfigFile(pybind11::handle filename) {
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::LoadLibconfigFile(PathStringFromPython(filename));
    return instance;
  }

  static ConfigWrapper LoadLibconfigString(std::string_view lcfg_str) {
    ConfigWrapper instance{};
    instance.cfg_ = wzkcfg::LoadLibconfigString(lcfg_str);
    return instance;
  }

  static ConfigWrapper FromPyDict(const pybind11::dict &d) {
    ConfigWrapper instance{};
    for (std::pair<pybind11::handle, pybind11::handle> item : d) {
      if (!pybind11::isinstance<pybind11::str>(item.first)) {
        std::string msg{"Dictionary keys must be strings, but got `"};
        msg += pybind11::cast<std::string>(item.first.attr("__class__").attr("__name__"));
        msg += "`!";
        throw wzkcfg::TypeError{msg};
      }
      const std::string key = item.first.cast<std::string>();

      if (!wzkcfg::IsValidKey(key, /*allow_dots=*/false)) {
        std::string msg{"Dictionary key `"};
        msg += key;
        msg += "` is not a valid parameter name! Only alpha-numeric characters, hyphen and underscore are allowed.";
        throw wzkcfg::TypeError{msg};
      }

      if (pybind11::isinstance<pybind11::str>(item.second)) {
        instance.SetString(key, item.second.cast<std::string>());
      } else if (pybind11::isinstance<pybind11::bool_>(item.second)) {
        instance.SetBoolean(key, item.second.cast<bool>());
      } else if (pybind11::isinstance<pybind11::int_>(item.second)) {
        instance.SetInteger64(key, item.second.cast<int64_t>());
      } else if (pybind11::isinstance<pybind11::float_>(item.second)) {
        instance.SetDouble(key, item.second.cast<double>());
      } else if (pybind11::isinstance<pybind11::list>(item.second)) {
        instance.SetList(key, item.second.cast<pybind11::list>());
      } else if (pybind11::isinstance<ConfigWrapper>(item.second)) {
        instance.SetGroup(key, item.second.cast<ConfigWrapper>());
      } else if (pybind11::isinstance<pybind11::dict>(item.second)) {
        instance.SetGroup(key, FromPyDict(item.second.cast<pybind11::dict>()));
      } else {
        const std::string tp =
          pybind11::cast<std::string>(item.second.attr("__class__").attr("__name__"));
        if (tp.compare("date") == 0) {
          instance.SetDate(key, item.second);
        } else if (tp.compare("time") == 0) {
          instance.SetTime(key, item.second);
        } else if (tp.compare("datetime") == 0) {
          instance.SetDateTime(key, item.second);
        } else {
          std::string msg{"Cannot convert a python object of type `"};
          msg += tp;
          msg += "` to a configuration parameter. Check dictionary key `";
          msg += key;
          msg += "`!";
          throw wzkcfg::TypeError{msg};
        }
      }
    }
    return instance;
  }

  std::string ToTOMLString() const { return cfg_.ToTOML(); }

  std::string ToJSONString() const { return cfg_.ToJSON(); }

  std::string ToYAMLString() const { return cfg_.ToYAML(); }

  std::string ToLibconfigString() const { return cfg_.ToLibconfig(); }

  bool Equals(const ConfigWrapper &other) const {
    return cfg_.Equals(other.cfg_);
  }

  bool Empty() const { return cfg_.Empty(); }

  bool Contains(std::string_view key) const { return cfg_.Contains(key); }

  std::size_t Size() const { return cfg_.Size(); }

  std::size_t ParameterSize(std::string_view key) const {
    return cfg_.Size(key);
  }

  wzkcfg::ConfigType Type(std::string_view key) const { return cfg_.Type(key); }

  wzkcfg::Configuration GetGroup(std::string_view key) const {
    return cfg_.GetGroup(key);
  }

  void ReplaceConfig(wzkcfg::Configuration &&c) { cfg_ = std::move(c); }
  void ReplaceConfig(const wzkcfg::Configuration &c) { cfg_ = c; }

  //---------------------------------------------------------------------------
  // Boolean

  void SetBoolean(std::string_view key, bool value) {
    cfg_.SetBoolean(key, value);
  }

  bool GetBoolean(std::string_view key) const { return cfg_.GetBoolean(key); }

  bool GetBooleanOr(std::string_view key, bool default_value) const {
    return cfg_.GetBooleanOr(key, default_value);
  }

  //---------------------------------------------------------------------------
  // Integer

  void SetInteger64(std::string_view key, int64_t value) {
    cfg_.SetInteger64(key, value);
  }

  int64_t GetInteger64(std::string_view key) const {
    return cfg_.GetInteger64(key);
  }

  int64_t GetInteger64Or(std::string_view key, int64_t default_value) const {
    return cfg_.GetInteger64Or(key, default_value);
  }

  //---------------------------------------------------------------------------
  // Floating point

  void SetDouble(std::string_view key, double value) {
    cfg_.SetDouble(key, value);
  }

  double GetDouble(std::string_view key) const { return cfg_.GetDouble(key); }

  double GetDoubleOr(std::string_view key, double default_value) const {
    return cfg_.GetDoubleOr(key, default_value);
  }

  //---------------------------------------------------------------------------
  // Strings

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

  //---------------------------------------------------------------------------
  // Date

  void SetDate(std::string_view key, pybind11::handle value) {
    const wzkcfg::date d = PyObjToDate(value);
    cfg_.SetDate(key, d);
  }

  pybind11::object GetDate(std::string_view key) const {
    return DateToPyObj(cfg_.GetDate(key));
  }

  pybind11::object GetDateOr(std::string_view key,
                             pybind11::handle default_value) const {
    const wzkcfg::date d = PyObjToDate(default_value);
    return DateToPyObj(cfg_.GetDateOr(key, d));
  }

  //---------------------------------------------------------------------------
  // Time

  void SetTime(std::string_view key, pybind11::handle value) {
    const wzkcfg::time t = PyObjToTime(value);
    cfg_.SetTime(key, t);
  }

  pybind11::object GetTime(std::string_view key) const {
    return TimeToPyObj(cfg_.GetTime(key));
  }

  pybind11::object GetTimeOr(std::string_view key,
                             pybind11::handle default_value) const {
    const wzkcfg::time t = PyObjToTime(default_value);
    return TimeToPyObj(cfg_.GetTimeOr(key, t));
  }

  //---------------------------------------------------------------------------
  // DateTime

  void SetDateTime(std::string_view key, pybind11::handle value) {
    const wzkcfg::date_time dt = PyObjToDateTime(value);
    cfg_.SetDateTime(key, dt);
  }

  pybind11::object GetDateTime(std::string_view key) const {
    return DateTimeToPyObj(cfg_.GetDateTime(key));
  }

  pybind11::object GetDateTimeOr(std::string_view key,
                                 pybind11::handle default_value) const {
    const wzkcfg::date_time dt = PyObjToDateTime(default_value);
    return DateTimeToPyObj(cfg_.GetDateTimeOr(key, dt));
  }

  //---------------------------------------------------------------------------
  // Group
  void SetGroup(std::string_view key, const ConfigWrapper &cfg) {
    cfg_.SetGroup(key, cfg.cfg_);
  }

  //---------------------------------------------------------------------------
  // List
  void SetList(std::string_view key, const pybind11::list &lst) {
    if (cfg_.Contains(key)) {
      cfg_.ClearList(key);
    } else {
      cfg_.CreateList(key);
    }
    // TODO wzk: CreateList should take care of append vs create

    for (pybind11::handle value : lst) {
      const std::string tp =
          pybind11::cast<std::string>(value.attr("__class__").attr("__name__"));

      if (pybind11::isinstance<pybind11::str>(value)) {
        cfg_.Append(key, value.cast<std::string>());
      } else if (pybind11::isinstance<pybind11::bool_>(value)) {
        cfg_.Append(key, value.cast<bool>());
      } else if (pybind11::isinstance<pybind11::int_>(value)) {
        cfg_.Append(key, value.cast<int64_t>());
      } else if (pybind11::isinstance<pybind11::float_>(value)) {
        cfg_.Append(key, value.cast<double>());
      } else if (pybind11::isinstance<pybind11::list>(value)) {
        std::size_t sz = cfg_.Size(key);
        cfg_.AppendList(key);
        std::string elem_key{key};
        elem_key += '[';
        elem_key += std::to_string(sz);
        elem_key += ']';
        SetList(elem_key, value.cast<pybind11::list>());
      } else if (pybind11::isinstance<ConfigWrapper>(value)) {
        cfg_.Append(key, value.cast<ConfigWrapper>().cfg_);
      } else if (pybind11::isinstance<pybind11::dict>(value)) {
        cfg_.Append(key, FromPyDict(value.cast<pybind11::dict>()).cfg_);
      } else {
        if (tp.compare("date") == 0) {
          cfg_.Append(key, PyObjToDate(value));
        } else if (tp.compare("time") == 0) {
          cfg_.Append(key, PyObjToTime(value));
        } else if (tp.compare("datetime") == 0) {
          cfg_.Append(key, PyObjToDateTime(value));
        } else {
          std::string msg{"Cannot append a python object of type `"};
          msg += tp;
          msg += "` to list `";
          msg += key;
          msg += "`!";
          throw wzkcfg::TypeError{msg};
        }
      }
    }
  }

  //---------------------------------------------------------------------------
  // Special functions
  std::vector<std::string> ListParameterNames(
      bool include_array_entries) const {
    return cfg_.ListParameterNames(include_array_entries, true);
  }

  std::vector<std::string> Keys() const {
    return cfg_.ListParameterNames(false, false);
  }

  bool ReplacePlaceholders(
      const std::vector<std::pair<std::string_view, std::string_view>>
          &replacements) {
    return cfg_.ReplaceStringPlaceholders(replacements);
  }

  void LoadNested(std::string_view key) { cfg_.LoadNestedConfiguration(key); }

  bool AdjustRelativePaths(pybind11::handle base_path,
                           const std::vector<std::string_view> &parameters) {
    cfg_.AdjustRelativePaths(PathStringFromPython(base_path), parameters);
  }

 private:
  wzkcfg::Configuration cfg_{};
};

inline pybind11::object GroupToPyObj(
    const pybind11::class_<ConfigWrapper> &pycfg,
    const wzkcfg::Configuration &wcfg) {
  pybind11::object obj = pycfg();
  auto *ptr = obj.cast<ConfigWrapper *>();
  ptr->ReplaceConfig(wcfg);
  return obj;
}

inline pybind11::list ListToPyList(const pybind11::class_<ConfigWrapper> &pycfg,
                                   const ConfigWrapper &wcfg,
                                   std::string_view key) {
  // TODO nice-to-have: create a custom readonly_list, so users can't run into
  // bugs like: cfg['lst'][3] = 'this will NOT change the parameter!'

  pybind11::list lst{};
  const std::size_t num_el = wcfg.ParameterSize(key);

  for (std::size_t idx = 0; idx < num_el; ++idx) {
    std::string elem_key{key};
    elem_key += '[';
    elem_key += std::to_string(idx);
    elem_key += ']';

    switch (wcfg.Type(elem_key)) {
      case wzkcfg::ConfigType::Boolean:
        lst.append(wcfg.GetBoolean(elem_key));
        break;

      case wzkcfg::ConfigType::Integer:
        lst.append(wcfg.GetInteger64(elem_key));
        break;

      case wzkcfg::ConfigType::FloatingPoint:
        lst.append(wcfg.GetDouble(elem_key));
        break;

      case wzkcfg::ConfigType::String:
        lst.append(wcfg.GetString(elem_key));
        break;

      case wzkcfg::ConfigType::List:
        lst.append(ListToPyList(pycfg, wcfg, elem_key));
        break;

      case wzkcfg::ConfigType::Group:
        lst.append(GroupToPyObj(pycfg, wcfg.GetGroup(elem_key)));
        break;

      case wzkcfg::ConfigType::Date:
        lst.append(wcfg.GetDate(elem_key));
        break;

      case wzkcfg::ConfigType::Time:
        lst.append(wcfg.GetTime(elem_key));
        break;

      case wzkcfg::ConfigType::DateTime:
        lst.append(wcfg.GetDateTime(elem_key));
    }
  }
  return lst;
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
                                    ? wzkcfg::ConfigType::Integer
                                    : wzkcfg::ConfigType::FloatingPoint;
    const auto expected = cfg.Contains(key) ? cfg.Type(key) : value_type;

    if (expected == wzkcfg::ConfigType::Integer) {
      cfg.SetInteger64(key, wzkcfg::checked_numcast<int64_t>(value));
    } else if (expected == wzkcfg::ConfigType::FloatingPoint) {
      cfg.SetDouble(key, wzkcfg::checked_numcast<double>(value));
    } else {
      std::string msg{"Changing the type is not allowed. Parameter `"};
      msg += key;
      msg += "` is `";
      msg += wzkcfg::ConfigTypeToString(expected);
      msg += "`, but input value is of type `";
      msg += wzkcfg::TypeName<TIn>();
      msg += "!";
      throw wzkcfg::TypeError{msg};
    }
  } else if constexpr (std::is_same_v<TSet, std::string>) {
    static_assert(std::is_same_v<TIn, std::string>);
    cfg.SetString(key, value);
  } else if constexpr (std::is_same_v<TSet, wzkcfg::date>) {
    cfg.SetDate(key, value);
  } else if constexpr (std::is_same_v<TSet, wzkcfg::time>) {
    cfg.SetTime(key, value);
  } else if constexpr (std::is_same_v<TSet, wzkcfg::date_time>) {
    cfg.SetDateTime(key, value);
  } else {
    throw std::runtime_error(
        "Setting this type is not yet supported!");  // TODO
  }
}

template <typename T>
T ExtractPythonNumber(pybind11::handle obj, const std::string &err_msg) {
  if (pybind11::isinstance<pybind11::int_>(obj)) {
    return wzkcfg::checked_numcast<T>(obj.cast<int64_t>());
  }

  if (pybind11::isinstance<pybind11::float_>(obj)) {
    return wzkcfg::checked_numcast<T>(obj.cast<double>());
  }

  throw wzkcfg::TypeError(err_msg);
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

          case wzkcfg::ConfigType::List:
            return ListToPyList(cfg, self, key);

          case wzkcfg::ConfigType::Group:
            return GroupToPyObj(cfg, self.GetGroup(key));

          case wzkcfg::ConfigType::Date:
            return self.GetDate(key);

          case wzkcfg::ConfigType::Time:
            return self.GetTime(key);

          case wzkcfg::ConfigType::DateTime:
            return self.GetDateTime(key);

            // default:
            //   // TODO
            //   throw std::runtime_error(
            //       "Accessing other types (list, groups, date, ...) is not yet
            //       " "implemented!");
        }
        return pybind11::none();
      },
      "Returns a copy of the parameter value.", pybind11::arg("key"));

  // type change is not supported
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
            case wzkcfg::ConfigType::Boolean:
              if (pybind11::isinstance<pybind11::bool_>(value)) {
                GenericScalarSetterUtil<bool>(self, key, value.cast<bool>());
                return;
              }
              break;

            case wzkcfg::ConfigType::Integer:
              GenericScalarSetterUtil<int64_t>(
                  self, key, ExtractPythonNumber<int64_t>(value, err_msg));
              return;

            case wzkcfg::ConfigType::FloatingPoint:
              GenericScalarSetterUtil<double>(
                  self, key, ExtractPythonNumber<double>(value, err_msg));
              return;

            case wzkcfg::ConfigType::String:
              if (pybind11::isinstance<pybind11::str>(value)) {
                GenericScalarSetterUtil<std::string>(self, key,
                                                     value.cast<std::string>());
                return;
              }
              break;

            case wzkcfg::ConfigType::List:
              if (pybind11::isinstance<pybind11::list>(value)) {
                self.SetList(key, value.cast<pybind11::list>());
                return;
              }
              break;

            case wzkcfg::ConfigType::Group:
              if (pybind11::isinstance<pybind11::dict>(value)) {
                self.SetGroup(key, ConfigWrapper::FromPyDict(
                                       value.cast<pybind11::dict>()));
              } else if (pybind11::isinstance<ConfigWrapper>(value)) {
                self.SetGroup(key, value.cast<ConfigWrapper>());
                return;
              }
              break;

            case wzkcfg::ConfigType::Date:
              GenericScalarSetterUtil<wzkcfg::date>(self, key, value);
              return;

            case wzkcfg::ConfigType::Time:
              GenericScalarSetterUtil<wzkcfg::time>(self, key, value);
              return;

            case wzkcfg::ConfigType::DateTime:
              GenericScalarSetterUtil<wzkcfg::date_time>(self, key, value);
              return;
          }
          throw wzkcfg::TypeError(err_msg);
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
          } else if (pybind11::isinstance<pybind11::list>(value)) {
            self.SetList(key, value.cast<pybind11::list>());
          } else if (pybind11::isinstance<pybind11::dict>(value)) {
            self.SetGroup(
                key, ConfigWrapper::FromPyDict(value.cast<pybind11::dict>()));
          } else if (pybind11::isinstance<ConfigWrapper>(value)) {
            self.SetGroup(key, value.cast<ConfigWrapper>());
          } else {
            if (tp.compare("date") == 0) {
              GenericScalarSetterUtil<wzkcfg::date>(self, key, value);
            } else if (tp.compare("time") == 0) {
              GenericScalarSetterUtil<wzkcfg::time>(self, key, value);
            } else if (tp.compare("datetime") == 0) {
              GenericScalarSetterUtil<wzkcfg::date_time>(self, key, value);
            } else {
              std::string msg{"Creating a new parameter (at key `"};
              msg += key;
              msg += "`) from python type `";
              msg += tp;
              msg += "` is not supported!";

              throw wzkcfg::TypeError(msg);
            }
          }
        }
      },
      "Sets the parameter value.", pybind11::arg("key"),
      pybind11::arg("value"));
  // TODO docstr
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
  cfg.def("load_nested_toml", &ConfigWrapper::LoadNested, doc_string.c_str(),
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

         print(cfg.to_toml_str())
      )doc";
  cfg.def("adjust_relative_paths", &ConfigWrapper::AdjustRelativePaths,
          doc_string.c_str(), pybind11::arg("base_path"),
          pybind11::arg("parameters"));
}
}  // namespace detail

inline void RegisterConfiguration(pybind11::module &m) {
  const std::string module_name = m.attr("__name__").cast<std::string>();
  const std::string config_name = std::string{module_name} + ".Configuration";

  std::string doc_string{};
  std::ostringstream doc_stream;
  doc_string = R"doc(
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

         from pyzeugkiste import config

         cfg = config.load_toml_str("""
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
         cfg.get_str('my_str')    # Alternative access

         cfg.get_int_or('unknown', -1)  # Allow default value

         cfg['unknown']      # Raises a KeyError
         cfg.get_int('str')  # Raises a TypeError

         cfg.replace_placeholders([
             ('%USR%', 'whoami'),
             ('%OUT%', '/path/to/output')])

         cfg.adjust_relative_paths(
             '/path/to/workdir',
             ['network_config', 'disk.*path'])

         cfg.load_nested_toml('disk.network_config')
         cfg['disk.network_config']  # Is now a group/dictionary

         print(cfg.to_toml_str())
    )doc";

  pybind11::class_<detail::ConfigWrapper> cfg(m, "Configuration",
                                              doc_string.c_str());

  cfg.def(pybind11::init<>(), "Creates an empty configuration.");

  //---------------------------------------------------------------------------
  // General members/operators
  cfg.def("empty", &detail::ConfigWrapper::Empty,
          "Checks if this configuration has any parameters set.");

  // Equality checks
  cfg.def(
      "__eq__",
      [](const detail::ConfigWrapper &a,
         const detail::ConfigWrapper &b) -> bool { return a.Equals(b); },
      "Checks for equality.\n\nReturns ``True`` if both configs contain the "
      "exact same configuration, *i.e.* keys, corresponding data types and\n"
      "values.",
      pybind11::arg("other"));

  cfg.def(
      "__ne__",
      [](const detail::ConfigWrapper &a,
         const detail::ConfigWrapper &b) -> bool { return !a.Equals(b); },
      "Checks for inequality, see :meth:`__eq__` for details.",
      pybind11::arg("other"));

  cfg.def("__str__",
          [cfg](const detail::ConfigWrapper &c) {
            std::ostringstream s;
            const std::string modname =
                cfg.attr("__module__").cast<std::string>();
            if (!modname.empty()) {
              s << modname << '.';
            }

            const std::size_t sz = c.Size();
            s << "Configuration(" << sz
              << ((sz == 1) ? " parameter" : " parameters") << ')';
            return s.str();
          })
      .def("__repr__", [](const detail::ConfigWrapper &c) {
        return c.ToTOMLString();
        // return c.attr("__name__").cast<std::string>();
        //  return cfg.attr("__name__").cast<std::string>();
        //  std::ostringstream s;
        //  s << m.attr("__name__").cast<std::string>() << ".Configuration()";
        //  // TODO x parameters, ...
        //  return s.str();
      });

  cfg.def("__contains__", &detail::ConfigWrapper::Contains,
          "Checks if the given key (fully-qualified parameter name) exists.",
          pybind11::arg("key"));

  cfg.def("__len__", &detail::ConfigWrapper::Size,
          "Returns the number of parameters (key-value pairs) in this "
          "configuration.");

  //---------------------------------------------------------------------------
  // Loading a configuration

  doc_string = R"doc(
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
  m.def("load", &detail::ConfigWrapper::LoadFile, doc_string.c_str(),
        pybind11::arg("filename"));

  // TODO document exceptions
  m.def("load_toml_str", &detail::ConfigWrapper::LoadTOMLString,
        "Loads the configuration from a `TOML <https://toml.io/en/>`__ string.",
        pybind11::arg("toml_str"));

  m.def("load_toml_file", &detail::ConfigWrapper::LoadTOMLFile,
        "Loads the configuration from a `TOML <https://toml.io/en/>`__ file.",
        pybind11::arg("filename"));

  // TODO NullValuePolicy
  m.def(
      "load_json_str", &detail::ConfigWrapper::LoadJSONString,
      "Loads the configuration from a `JSON <https://www.json.org/>`__ string.",
      pybind11::arg("json_str"));

  // TODO NullValuePolicy
  m.def("load_json_file", &detail::ConfigWrapper::LoadJSONFile,
        "Loads the configuration from a `JSON <https://www.json.org/>`__ file.",
        pybind11::arg("filename"));

  // TODO libconfig (doc: only available if built with libconfig support)
  // TODO enable automatically if libconfig++ is installed or enable via
  //   install extras?
  m.def("load_libconfig_str", &detail::ConfigWrapper::LoadLibconfigString,
        "Loads the configuration from a `Libconfig "
        "<http://hyperrealm.github.io/libconfig/>`__ "
        "string if `libconfig++` is available.",
        pybind11::arg("cfg_str"));

  m.def("load_libconfig_file", &detail::ConfigWrapper::LoadLibconfigFile,
        "Loads the configuration from a `Libconfig "
        "<http://hyperrealm.github.io/libconfig/>`__ file.",
        pybind11::arg("filename"));

  //---------------------------------------------------------------------------
  // Serializing
  cfg.def("to_toml", &detail::ConfigWrapper::ToTOMLString,
          "Returns a `TOML <https://toml.io/>`__-formatted representation "
          "of this configuration.");

  cfg.def("to_json", &detail::ConfigWrapper::ToJSONString,
          "Returns a `JSON <https://www.json.org/>`__-formatted representation "
          "of this configuration.\n\nNote that date/time parameters will be "
          "replaced by their string representation.");

  // TODO should be supported always (change in wzk!)
  cfg.def("to_libconfig", &detail::ConfigWrapper::ToLibconfigString,
          "Returns a `Libconfig "
          "<http://hyperrealm.github.io/libconfig/>`__-formatted "
          "representation of this configuration.");

  cfg.def("to_yaml", &detail::ConfigWrapper::ToYAMLString,
          "Returns a `YAML <https://yaml.org/>`__-formatted representation "
          "of this configuration.");
  // TODO state currently supported version + auto-replacements (date?)

  //---------------------------------------------------------------------------
  // Getter/Setter

  detail::RegisterScalarAccess(cfg);
  detail::RegisterGenericAccess(cfg);

  //---------------------------------------------------------------------------
  // Getting/Setting lists/tuples/pairs

  // TODO lists
  // TODO tuples/pairs/points - not needed (?)
  // TODO nested lists/numpy arrays (matrices)

  //---------------------------------------------------------------------------
  // Special utils
  detail::RegisterConfigUtilities(cfg);

  //---------------------------------------------------------------------------
  // Register exceptions
  // The corresponding python module __init__ will override the __module__
  // string of these exceptions. Otherwise, if raised they might confuse the
  // user (due to the binding-internal module name "_core._cfg")

  pybind11::register_local_exception<wzkcfg::KeyError>(m, "KeyError",
                                                       PyExc_KeyError);
  pybind11::register_local_exception<wzkcfg::TypeError>(m, "TypeError",
                                                        PyExc_TypeError);
  pybind11::register_local_exception<wzkcfg::ValueError>(m, "ValueError",
                                                         PyExc_ValueError);
  pybind11::register_local_exception<wzkcfg::ParseError>(m, "ParseError",
                                                         PyExc_RuntimeError);

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
