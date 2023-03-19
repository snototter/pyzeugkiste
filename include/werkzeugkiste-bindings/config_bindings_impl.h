#ifndef WERKZEUGKISTE_BINDINGS_CONFIG_IMPL_H
#define WERKZEUGKISTE_BINDINGS_CONFIG_IMPL_H

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
class ConfigWrapper {
 public:
  //---------------------------------------------------------------------------
  // Loading / Deserialization

  static ConfigWrapper LoadFile(pybind11::handle filename) {
    ConfigWrapper instance{};
    instance.cfg_ = werkzeugkiste::config::LoadFile(PyObjToPathString(filename));
    return instance;
  }

  static ConfigWrapper LoadTOMLFile(pybind11::handle filename) {
    ConfigWrapper instance{};
    instance.cfg_ = werkzeugkiste::config::LoadTOMLFile(PyObjToPathString(filename));
    return instance;
  }

  static ConfigWrapper LoadTOMLString(std::string_view toml_str) {
    ConfigWrapper instance{};
    instance.cfg_ = werkzeugkiste::config::LoadTOMLString(toml_str);
    return instance;
  }

  static ConfigWrapper LoadJSONFile(pybind11::handle filename) {
    // TODO support NullValuePolicy
    ConfigWrapper instance{};
    instance.cfg_ = werkzeugkiste::config::LoadJSONFile(PyObjToPathString(filename));
    return instance;
  }

  static ConfigWrapper LoadJSONString(std::string_view json_str) {
    // TODO support NullValuePolicy
    ConfigWrapper instance{};
    instance.cfg_ = werkzeugkiste::config::LoadJSONString(json_str);
    return instance;
  }

  static ConfigWrapper LoadLibconfigFile(pybind11::handle filename) {
    ConfigWrapper instance{};
    instance.cfg_ = werkzeugkiste::config::LoadLibconfigFile(PyObjToPathString(filename));
    return instance;
  }

  static ConfigWrapper LoadLibconfigString(std::string_view lcfg_str) {
    ConfigWrapper instance{};
    instance.cfg_ = werkzeugkiste::config::LoadLibconfigString(lcfg_str);
    return instance;
  }

  static ConfigWrapper FromPyDict(const pybind11::dict &d) {
    ConfigWrapper instance{};
    for (std::pair<pybind11::handle, pybind11::handle> item : d) {
      if (!pybind11::isinstance<pybind11::str>(item.first)) {
        std::string msg{"Dictionary keys must be strings, but got `"};
        msg += pybind11::cast<std::string>(item.first.attr("__class__").attr("__name__"));
        msg += "`!";
        throw werkzeugkiste::config::TypeError{msg};
      }
      const std::string key = item.first.cast<std::string>();

      if (!werkzeugkiste::config::IsValidKey(key, /*allow_dots=*/false)) {
        std::string msg{"Dictionary key `"};
        msg += key;
        msg += "` is not a valid parameter name! Only alpha-numeric characters, hyphen and underscore are allowed.";
        throw werkzeugkiste::config::TypeError{msg};
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
          throw werkzeugkiste::config::TypeError{msg};
        }
      }
    }
    return instance;
  }

  //---------------------------------------------------------------------------
  // Serialization

  std::string ToTOMLString() const { return cfg_.ToTOML(); }

  std::string ToJSONString() const { return cfg_.ToJSON(); }

  std::string ToYAMLString() const { return cfg_.ToYAML(); }

  std::string ToLibconfigString() const { return cfg_.ToLibconfig(); }

  pybind11::object ToDict() const {
    return GroupToPyObj(nullptr, cfg_);
  }
  
  //---------------------------------------------------------------------------
  // Operators/Utils/Basics

  bool Equals(const ConfigWrapper &other) const {
    return cfg_.Equals(other.cfg_);
  }

  bool Empty() const { return cfg_.Empty(); }

  bool Contains(std::string_view key) const { return cfg_.Contains(key); }

  std::size_t Size() const { return cfg_.Size(); }

  std::size_t ParameterSize(std::string_view key) const {
    return cfg_.Size(key);
  }

  werkzeugkiste::config::ConfigType Type(std::string_view key) const { return cfg_.Type(key); }

  void ReplaceConfig(werkzeugkiste::config::Configuration &&c) { cfg_ = std::move(c); }
  void ReplaceConfig(const werkzeugkiste::config::Configuration &c) { cfg_ = c; }

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
    const werkzeugkiste::config::date d = PyObjToDate(value);
    cfg_.SetDate(key, d);
  }

  pybind11::object GetDate(std::string_view key) const {
    return DateToPyObj(cfg_.GetDate(key));
  }

  pybind11::object GetDateOr(std::string_view key,
                             pybind11::handle default_value) const {
    const werkzeugkiste::config::date d = PyObjToDate(default_value);
    return DateToPyObj(cfg_.GetDateOr(key, d));
  }

  //---------------------------------------------------------------------------
  // Time

  void SetTime(std::string_view key, pybind11::handle value) {
    const werkzeugkiste::config::time t = PyObjToTime(value);
    cfg_.SetTime(key, t);
  }

  pybind11::object GetTime(std::string_view key) const {
    return TimeToPyObj(cfg_.GetTime(key));
  }

  pybind11::object GetTimeOr(std::string_view key,
                             pybind11::handle default_value) const {
    const werkzeugkiste::config::time t = PyObjToTime(default_value);
    return TimeToPyObj(cfg_.GetTimeOr(key, t));
  }

  //---------------------------------------------------------------------------
  // DateTime

  void SetDateTime(std::string_view key, pybind11::handle value) {
    const werkzeugkiste::config::date_time dt = PyObjToDateTime(value);
    cfg_.SetDateTime(key, dt);
  }

  pybind11::object GetDateTime(std::string_view key) const {
    return DateTimeToPyObj(cfg_.GetDateTime(key));
  }

  pybind11::object GetDateTimeOr(std::string_view key,
                                 pybind11::handle default_value) const {
    const werkzeugkiste::config::date_time dt = PyObjToDateTime(default_value);
    return DateTimeToPyObj(cfg_.GetDateTimeOr(key, dt));
  }

  //---------------------------------------------------------------------------
  // Group
  void SetGroup(std::string_view key, const ConfigWrapper &cfg) {
    cfg_.SetGroup(key, cfg.cfg_);
  }

  werkzeugkiste::config::Configuration GetGroup(std::string_view key) const {
    return cfg_.GetGroup(key);
  }

  const werkzeugkiste::config::Configuration &GetRootGroup() const {
    return cfg_;
  }

  //---------------------------------------------------------------------------
  // List
  void SetList(std::string_view key, const pybind11::list &lst) {
    if (cfg_.Contains(key)) {
      cfg_.ClearList(key);
    } else {
      cfg_.CreateList(key);
    }

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
          throw werkzeugkiste::config::TypeError{msg};
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
    cfg_.AdjustRelativePaths(PyObjToPathString(base_path), parameters);
  }

 private:
  werkzeugkiste::config::Configuration cfg_{};
};



inline werkzeugkiste::config::date PyObjToDateUnchecked(pybind11::handle obj) {
  const int year = obj.attr("year").cast<int>();
  const int month = obj.attr("month").cast<int>();
  const int day = obj.attr("day").cast<int>();
  return werkzeugkiste::config::date{static_cast<uint_fast16_t>(year),
                      static_cast<uint_fast8_t>(month),
                      static_cast<uint_fast8_t>(day)};
}


inline werkzeugkiste::config::time PyObjToTimeUnchecked(pybind11::handle obj) {
  const int hour = obj.attr("hour").cast<int>();
  const int minute = obj.attr("minute").cast<int>();
  const int second = obj.attr("second").cast<int>();
  const int microsec = obj.attr("microsecond").cast<int>();
  return werkzeugkiste::config::time{static_cast<uint_fast8_t>(hour),
                      static_cast<uint_fast8_t>(minute),
                      static_cast<uint_fast8_t>(second),
                      static_cast<uint_fast32_t>(microsec * 1000)};
}


inline werkzeugkiste::config::date PyObjToDate(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return werkzeugkiste::config::date{obj.cast<std::string>()};
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
  throw werkzeugkiste::config::TypeError(msg);
}


inline werkzeugkiste::config::time PyObjToTime(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return werkzeugkiste::config::time{obj.cast<std::string>()};
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
  throw werkzeugkiste::config::TypeError(msg);
}


inline werkzeugkiste::config::date_time PyObjToDateTime(pybind11::handle obj) {
  if (pybind11::isinstance<pybind11::str>(obj)) {
    return werkzeugkiste::config::date_time{obj.cast<std::string>()};
  }

  // We need to ensure that the PyDateTime import is initialized.
  // Or prepare for segfaults.
  if (!PyDateTimeAPI) {
    PyDateTime_IMPORT;
  }

  if (PyDateTime_Check(obj.ptr())) {
    // Object is datetime.datetime or subtype
    const werkzeugkiste::config::date d = PyObjToDateUnchecked(obj);
    const werkzeugkiste::config::time t = PyObjToTimeUnchecked(obj);

    if (obj.attr("tzinfo").is_none()) {
      return werkzeugkiste::config::date_time{d, t};
    } else {
      const auto pyoffset_sec = obj.attr("utcoffset")().attr("total_seconds")();
      const auto offset_min =
          static_cast<int_fast16_t>(pyoffset_sec.cast<double>() / 60.0);
      return werkzeugkiste::config::date_time{d, t, werkzeugkiste::config::time_offset{offset_min}};
    }
  }

  const std::string tp =
      pybind11::cast<std::string>(obj.attr("__class__").attr("__name__"));
  std::string msg{"Cannot convert python type `"};
  msg += tp;
  msg += "` to `werkzeugkiste::date_time`!";
  throw werkzeugkiste::config::TypeError(msg);
}


inline pybind11::object DateToPyObj(const werkzeugkiste::config::date &d) {
  auto pydatetime = pybind11::module::import("datetime");
  return pydatetime.attr("date")(d.year, d.month, d.day);
}

inline pybind11::object TimeToPyObj(const werkzeugkiste::config::time &t) {
  auto pydatetime = pybind11::module::import("datetime");
  return pydatetime.attr("time")(t.hour, t.minute, t.second,
                                 t.nanosecond / 1000);
}

inline pybind11::object DateTimeToPyObj(const werkzeugkiste::config::date_time &dt) {
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


/// If the class handle is provided, the configuration group will be returned
/// as a ConfigWrapper instance (which is available in python).
/// Otherwise, the configuration group will be returned as a pure python dict.
inline pybind11::object GroupToPyObj(
    const pybind11::class_<ConfigWrapper> *cls_handle,
    const werkzeugkiste::config::Configuration &cfg) {
  if (cls_handle) {
    pybind11::object obj = (*cls_handle)();
    auto *ptr = obj.cast<ConfigWrapper *>();
    ptr->ReplaceConfig(cfg);
    return obj;
  }

  pybind11::dict d;
  const std::vector<std::string> keys = cfg.ListParameterNames(false, false);
  for (const std::string &key : keys) {
    const char *dkey = key.c_str();
    switch (cfg.Type(key)) {
      case werkzeugkiste::config::ConfigType::Boolean:
        d[dkey] = cfg.GetBoolean(key);
        break;

      case werkzeugkiste::config::ConfigType::Integer:
        d[dkey] = cfg.GetInteger64(key);
        break;

      case werkzeugkiste::config::ConfigType::FloatingPoint:
        d[dkey] = cfg.GetDouble(key);
        break;

      case werkzeugkiste::config::ConfigType::String:
        d[dkey] = cfg.GetString(key);
        break;

      case werkzeugkiste::config::ConfigType::Date:
        d[dkey] = DateToPyObj(cfg.GetDate(key));
        break;

      case werkzeugkiste::config::ConfigType::Time:
        d[dkey] = TimeToPyObj(cfg.GetTime(key));
        break;
      
      case werkzeugkiste::config::ConfigType::DateTime:
        d[dkey] = DateTimeToPyObj(cfg.GetDateTime(key));
        break;

      case werkzeugkiste::config::ConfigType::List:
        d[dkey] = ListToPyList(cls_handle, cfg, key);
        break;

      case werkzeugkiste::config::ConfigType::Group:
        d[dkey] = GroupToPyObj(cls_handle, cfg.GetGroup(key));
        break;
    }
  }
  return d;
}

inline pybind11::list ListToPyList(const pybind11::class_<ConfigWrapper> *cls_handle,
                                   const werkzeugkiste::config::Configuration &cfg,
                                   std::string_view key) {
  // TODO nice-to-have: create a custom readonly_list, so users can't run into
  // bugs like: cfg['lst'][3] = 'this will NOT change the parameter!'

  pybind11::list lst{};
  const std::size_t num_el = cfg.Size(key);

  for (std::size_t idx = 0; idx < num_el; ++idx) {
    std::string elem_key{key};
    elem_key += '[';
    elem_key += std::to_string(idx);
    elem_key += ']';

    switch (cfg.Type(elem_key)) {
      case werkzeugkiste::config::ConfigType::Boolean:
        lst.append(cfg.GetBoolean(elem_key));
        break;

      case werkzeugkiste::config::ConfigType::Integer:
        lst.append(cfg.GetInteger64(elem_key));
        break;

      case werkzeugkiste::config::ConfigType::FloatingPoint:
        lst.append(cfg.GetDouble(elem_key));
        break;

      case werkzeugkiste::config::ConfigType::String:
        lst.append(cfg.GetString(elem_key));
        break;

      case werkzeugkiste::config::ConfigType::List:
        lst.append(ListToPyList(cls_handle, cfg, elem_key));
        break;

      case werkzeugkiste::config::ConfigType::Group:
        lst.append(GroupToPyObj(cls_handle, cfg.GetGroup(elem_key)));
        break;

      case werkzeugkiste::config::ConfigType::Date:
        lst.append(DateToPyObj(cfg.GetDate(elem_key)));
        break;

      case werkzeugkiste::config::ConfigType::Time:
        lst.append(TimeToPyObj(cfg.GetTime(elem_key)));
        break;

      case werkzeugkiste::config::ConfigType::DateTime:
        lst.append(DateTimeToPyObj(cfg.GetDateTime(elem_key)));
    }
  }
  return lst;
}

inline std::string PyObjToPathString(pybind11::handle path) {
  if (pybind11::isinstance<pybind11::str>(path)) {
    return path.cast<std::string>();
  } else {
    return pybind11::str(path).cast<std::string>();
  }
}

}  // namespace werkzeugkiste::bindings::detail

#endif  // WERKZEUGKISTE_BINDINGS_CONFIG_IMPL_H