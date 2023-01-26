#include <sstream>
#include <stdexcept>
#include <string_view>

#include <pybind11/operators.h>
#include <pybind11/numpy.h>

#include <werkzeugkiste/geometry/vector.h>

//#include <werkzeugkiste-bindings/logging.h>

namespace werkzeugkiste::bindings {

//------------------------------------------------- Vector from tuple
/// Constructs a vector from a pybind11::tuple or pybind11::list.
template<typename Tp, std::size_t Dim> inline
werkzeugkiste::geometry::Vec<Tp, Dim> VecFromTupleOrList(
    const pybind11::tuple &tpl,
    std::string_view py_type,
    std::string_view module_name) {
  using VC = werkzeugkiste::geometry::Vec<Tp, Dim>;
  if (tpl.empty()) {
    return VC();
  }

  if (tpl.size() != Dim) {
    std::ostringstream s;
    s << "Cannot cast " << py_type << " with " << tpl.size()
      << " elements to `" << module_name << "." << VC::TypeName() << "`!";
    SPDLOG_ERROR(s.str());
    throw std::invalid_argument(s.str());
  }

  VC vec;
  for (std::size_t i = 0; i < Dim; ++i) {
    vec.val[i] = tpl[i].cast<Tp>();
  }
  return vec;
}



template<typename Tp, std::size_t Dim>
werkzeugkiste::geometry::Vec<Tp, Dim> VecFromList(
    const pybind11::list &lst,
    std::string_view module_name) {
  return VecFromTupleOrList<Tp, Dim>(
        lst.cast<pybind11::tuple>(), "list", module_name);
}


template<typename Tp, std::size_t Dim>
werkzeugkiste::geometry::Vec<Tp, Dim> VecFromTuple(
    const pybind11::tuple &tpl,
    std::string_view module_name) {
  return VecFromTupleOrList<Tp, Dim>(tpl, "tuple", module_name);
}


/// To support pickling, it is easier to serialize the vector
/// as a list. The other option would be to have explicit specializations
/// for each type and dimensionality, so we can create a pybind11::tuple via
/// pybind11::make_tuple...
template<typename Tp, std::size_t Dim>
pybind11::list VecToList(const werkzeugkiste::geometry::Vec<Tp, Dim> &vec) {
  pybind11::list lst;
  for (std::size_t i = 0; i < Dim; ++i) {
    lst.append(vec.val[i]);
  }
  return lst;
}

template <typename Tp, std::size_t Dim>
pybind11::tuple VecToTuple(const werkzeugkiste::geometry::Vec<Tp, Dim> &vec) {
  static_assert(
      (Dim >= 2) && (Dim <= 4),
      "Only implemented for 2D, 3D and 4D vectors!");

  if constexpr (Dim == 2) {
    return pybind11::make_tuple(vec.X(), vec.Y());
  }

  if constexpr (Dim == 3) {
    return pybind11::make_tuple(vec.X(), vec.Y(), vec.Z());
  }

  if constexpr(Dim == 4) {
    return pybind11::make_tuple(vec.X(), vec.Y(), vec.Z(), vec.W());
  }

  throw std::logic_error("VecToTuple not implemented for Dim > 4!");
}


/// Templated helper to convert a Python array to a Vec<...>, called
/// from VecFromArray()
template<typename ATp, typename VTp, std::size_t Dim> inline
werkzeugkiste::geometry::Vec<VTp, Dim> VecFromArrayT(
    const pybind11::array &arr,
    std::string_view module_name) {
  using VC = werkzeugkiste::geometry::Vec<VTp, Dim>;
  constexpr int dim_int = static_cast<int>(Dim);

  const int rows = arr.shape(0);
  const int cols = (arr.ndim() > 1) ? arr.shape(1) : 1;
  const int channels = (arr.ndim() == 3) ? arr.shape(2) : 1;
  const int row_stride = arr.strides(0) / sizeof(ATp);
  const int col_stride = (arr.ndim() > 1)
      ? (arr.strides(1) / sizeof(ATp)) : 1;
  const int channel_stride = (arr.ndim() == 3)
      ? (arr.strides(2) / sizeof(ATp)) : 1;

  if ((rows * cols * channels) != dim_int) {
    std::ostringstream s;
    s << "Cannot cast array with " << arr.size() << " elements to `"
      << module_name << '.' << VC::TypeName() << "`!";
    SPDLOG_ERROR(s.str());
    throw std::invalid_argument(s.str());
  }

  VC vec;
  int vec_idx = 0;
  const ATp *arr_row_ptr = reinterpret_cast<const ATp*>(arr.data());
  for (int row = 0; row < rows; ++row) {
    const ATp *arr_col_ptr = arr_row_ptr;
    for (int col = 0; col < cols; ++col) {
      const ATp *arr_chan_ptr = arr_col_ptr;
      for (int channel = 0; channel < channels; ++channel) {
        vec[vec_idx++] = static_cast<VTp>(*arr_chan_ptr);
        arr_chan_ptr += channel_stride;
      }
      arr_col_ptr += col_stride;
    }
    arr_row_ptr += row_stride;
  }
  return vec;
}


/// Allows casting a numpy.ndarray (view) with DIM elements to a
/// DIM-dimensional Vec<>
template<typename Tp, std::size_t Dim> inline
werkzeugkiste::geometry::Vec<Tp, Dim> VecFromArray(
    const pybind11::array &arr,
    std::string_view module_name) {
  using VC = werkzeugkiste::geometry::Vec<Tp, Dim>;

  if (arr.size() == 0) {
    return VC();
  }

  const pybind11::dtype dtype = arr.dtype();
  if (pybind11::isinstance<pybind11::array_t<uint8_t>>(arr)) {
    return VecFromArrayT<uint8_t, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<int8_t>>(arr)) {
    return VecFromArrayT<int8_t, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<int16_t>>(arr)) {
    return VecFromArrayT<int16_t, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<uint16_t>>(arr)) {
    return VecFromArrayT<uint16_t, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<int32_t>>(arr)) {
    return VecFromArrayT<int32_t, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<uint32_t>>(arr)) {
    return VecFromArrayT<uint32_t, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<int64_t>>(arr)) {
    return VecFromArrayT<int64_t, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<uint64_t>>(arr)) {
    return VecFromArrayT<uint64_t, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<float>>(arr)) {
    return VecFromArrayT<float, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<double>>(arr)) {
    return VecFromArrayT<double, Tp, Dim>(arr, module_name);
  } else if (pybind11::isinstance<pybind11::array_t<bool>>(arr)) {
    return VecFromArrayT<bool, Tp, Dim>(arr, module_name);
  } else {
    std::string s("Incompatible `dtype` (");
    s += pybind11::cast<std::string>(dtype.attr("name"));
    s += ", \"";
    const pybind11::list dt_descr = pybind11::cast<pybind11::list>(dtype.attr("descr"));
    for (std::size_t i = 0; i < dt_descr.size(); ++i) {
      // First element holds the optional name, second one holds the
      // type description we're interested in, check for example:
      // https://numpy.org/doc/stable/reference/generated/numpy.dtype.descr.html
      const pybind11::tuple td = pybind11::cast<pybind11::tuple>(dt_descr[i]);
      s += pybind11::cast<std::string>(td[1]);
      if (i < dt_descr.size() - 1) {
        s += "\", \"";
      }
    }
    s += "\") to construct a `";
    s += module_name;
    s += '.';
    s += VC::TypeName() + "`!";
    SPDLOG_ERROR(s);
    throw std::invalid_argument(s);
  }
}


/// Generic casting: from tuple/list/ndarray to Vec<>
template<typename Tp, std::size_t Dim>
werkzeugkiste::geometry::Vec<Tp, Dim> VecFromPyObject(
    const pybind11::object &object,
    std::string_view module_name) {
  using VC = werkzeugkiste::geometry::Vec<Tp, Dim>;

  if (pybind11::isinstance<pybind11::tuple>(object)) {
    return VecFromTupleOrList<Tp, Dim>(
          object.cast<pybind11::tuple>(), "tuple", module_name);
  }

  if (pybind11::isinstance<pybind11::list>(object)) {
    return VecFromTupleOrList<Tp, Dim>(
          object.cast<pybind11::tuple>(), "list", module_name);
  }

  if (pybind11::isinstance<pybind11::array>(object)) {
    return VecFromArray<Tp, Dim>(
          object.cast<pybind11::array>(), module_name);
  }

  const std::string type = pybind11::cast<std::string>(object.attr("__class__").attr("__name__"));
  std::ostringstream s;
  s << "Cannot cast `" << type << "` to `"
    << module_name << '.' << VC::TypeName()
    << "`. Only tuple, list and numpy.ndarray is supported!";
  SPDLOG_ERROR(s.str());
  throw std::invalid_argument(s.str());
}


/// Returns the vector's type name to be used in docstrings, optionally
/// refering to the python built-in to be used by the cross-referencing within
/// our RTD documentation.
template<typename Tp>
const char *PythonVecTypeName(bool rst);


template<> const char *PythonVecTypeName<double>(bool rst) {
  if (rst) {
    return ":class:`float`";
  } else {
    return "float:";
  }
}


template<> const char *PythonVecTypeName<int32_t>(bool rst) {
  if (rst) {
    return ":class:`int`";
  } else {
    return "int:";
  }
}


/// Needed to implicitly cast a Vec<> to a numpy array
template<typename Tp, std::size_t Dim>
pybind11::buffer_info VecInfo(werkzeugkiste::geometry::Vec<Tp, Dim> &vec) {
  return pybind11::buffer_info(
      &vec[0],
      sizeof(Tp), // Size of each element
      pybind11::format_descriptor<Tp>::format(), // Python struct-style format descriptor
      1,  // The buffer is 1-dimensional
      { Dim }, // Buffer dimensions
      { sizeof(Tp) } // Stride (in bytes) per dimension
  );
}


template<class V>
inline void RegisterVectorAdditionSubtraction(
    pybind11::class_<V> &vec,
    std::string_view module_name) {
  using value_type = typename V::value_type;
  using V_int32 = werkzeugkiste::geometry::Vec<value_type, V::ndim>;

  const std::string fqn_type = std::string(module_name) + "." + V::TypeName();
  const std::string fqn_int32 = std::string(module_name) + "." + V_int32::TypeName();

  //---------------------------------------------------------------------------
  // Addition "V + X"

  std::ostringstream doc;
  doc << "Element-wise addition: :class:`~" << fqn_type
      << "` = :class:`~" << fqn_type << "` + :class:`int`.";
  vec.def(
      "__add__",
      [](const V &self, int32_t rhs) -> V {
        // 32-bit integers can safely be cast into double precision types:
        return self + static_cast<value_type>(rhs);
      },
      doc.str().c_str(),
      pybind11::arg("scalar"));


  std::ostringstream().swap(doc);
  doc << "Element-wise addition: :class:`~" << fqn_type
      << "` = :class:`int` + :class:`~" << fqn_type << "`.";
  vec.def(
      "__radd__",
      [](const V &self, int32_t lhs) -> V {
        // 32-bit integers can safely be cast into double precision types:
        return self + static_cast<value_type>(lhs);
      },
      doc.str().c_str(),
      pybind11::arg("lhs"));


  // Vector + Vector (of the same type)
  std::ostringstream().swap(doc);
  doc << "Element-wise addition: :class:`~" << fqn_type
      << "` = :class:`~" << fqn_type << "` + :class:`~" << fqn_type << "`.";
  vec.def(
      "__add__",
      [](const V &self, const V &rhs) -> V {
        return self + rhs;
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  if constexpr (std::is_floating_point_v<value_type>) {
    // Allow implicit upcasting of int32 vectors
    std::ostringstream().swap(doc);
    doc << "Element-wise addition: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_type
        << "` + :class:`~" << fqn_int32 << "`.";
    vec.def(
        "__add__",
        [](const V &self, const V_int32 &rhs) -> V {
          V dbl = rhs.ToDouble();
          dbl += self;
          return dbl;
        },
        doc.str().c_str(),
        pybind11::arg("rhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise addition: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_int32
        << "` + :class:`~" << fqn_type << "`.";
    vec.def(
        "__radd__",
        [](const V &self, const V_int32 &lhs) -> V {
          V dbl = lhs.ToDouble();
          dbl += self;
          return dbl;
        },
        doc.str().c_str(),
        pybind11::arg("lhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise addition: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_type << "` + :class:`float`.";
    vec.def(
        "__add__",
        [](const V &self, double rhs) -> V {
          return self + rhs;
        },
        doc.str().c_str(),
        pybind11::arg("rhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise addition: :class:`~" << fqn_type
        << "` = :class:`float` + :class:`~" << fqn_type << "`.";
    vec.def(
        "__radd__",
        [](const V &self, double lhs) -> V {
          return self + lhs;
        },
        doc.str().c_str(),
        pybind11::arg("lhs"));
  }


  //---------------------------------------------------------------------------
  // Addition "V += X"

  std::ostringstream().swap(doc);
  doc << "In-place element-wise addition: :class:`~" << fqn_type
      << "` += :class:`int`.";
  vec.def(
      "__iadd__",
      [](V &self, int32_t rhs) -> V& {
        // 32-bit integers can safely be cast into double precision types:
        return self.AddScalar(static_cast<value_type>(rhs));
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  // Vector += Vector (of the same type)
  std::ostringstream().swap(doc);
  doc << "In-place element-wise addition: :class:`~" << fqn_type
      << "` += :class:`~" << fqn_type << "`.";
  vec.def(
      "__iadd__",
      [](V &self, const V &rhs) -> V& {
        return self.AddVector(rhs);
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  if constexpr (std::is_floating_point_v<value_type>) {
    // Allow implicit upcasting of int32 vectors
    std::ostringstream().swap(doc);
    doc << "In-place element-wise addition: :class:`~" << fqn_type
        << "` += :class:`~" << fqn_int32 << "`.";
    vec.def(
        "__iadd__",
        [](V &self, const V_int32 &rhs) -> V& {
          return self.AddVector(rhs.ToDouble());
        },
        doc.str().c_str(),
        pybind11::arg("rhs"));


    std::ostringstream().swap(doc);
    doc << "In-place element-wise addition: :class:`~" << fqn_type
        << "` += :class:`float`.";
    vec.def(
        "__iadd__",
        [](V &self, double rhs) -> V& {
          return self.AddScalar(rhs);
        },
        doc.str().c_str(),
        pybind11::arg("rhs"));
  }


  //---------------------------------------------------------------------------
  // Subtraction "V - X"

  std::ostringstream().swap(doc);
  doc << "Element-wise subtraction: :class:`~" << fqn_type
      << "` = :class:`~" << fqn_type << "` - :class:`int`.";
  vec.def(
      "__sub__",
      [](const V &self, int32_t rhs) -> V {
        // 32-bit integers can safely be cast into double precision types:
        return self - static_cast<value_type>(rhs);
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  std::ostringstream().swap(doc);
  doc << "Element-wise subtraction: :class:`~" << fqn_type
      << "` = :class:`int` - :class:`~" << fqn_type << "`.";
  vec.def(
      "__rsub__",
      [](const V &self, int32_t lhs) -> V {
        // 32-bit integers can safely be cast into double precision types:
        return V::All(static_cast<value_type>(lhs)) - self;
      },
      doc.str().c_str(),
      pybind11::arg("lhs"));


  // Vector - Vector (of the same type)
  std::ostringstream().swap(doc);
  doc << "Element-wise subtraction: :class:`~" << fqn_type
      << "` = :class:`~" << fqn_type
      << "` - :class:`~" << fqn_type << "`.";
  vec.def(
      "__sub__",
      [](const V &self, const V &rhs) -> V {
        return self - rhs;
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  if constexpr (std::is_floating_point_v<value_type>) {
    // Allow implicit upcasting of int32 vectors
    std::ostringstream().swap(doc);
    doc << "Element-wise subtraction: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_type
        << "` - :class:`~" << fqn_int32 << "`.";
    vec.def(
        "__sub__",
        [](const V &self, const V_int32 &rhs) -> V {
          V dbl = rhs.ToDouble().Negate();
          dbl += self;
          return dbl;
        },
        doc.str().c_str(),
        pybind11::arg("rhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise subtraction: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_int32
        << "` - :class:`~" << fqn_type << "`.";
    vec.def(
        "__rsub__",
        [](const V &self, const V_int32 &lhs) -> V {
          return lhs.ToDouble() - self;
        },
        doc.str().c_str(),
        pybind11::arg("lhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise subtraction: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_type
        << "` - :class:`float`.";
    vec.def(
        "__sub__",
        [](const V &self, double rhs) -> V {
          return self - rhs;
        },
        doc.str().c_str(),
        pybind11::arg("rhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise subtraction: :class:`~" << fqn_type
        << "` = :class:`float` - :class:`~"
        << fqn_type << "`.";
    vec.def(
        "__rsub__",
        [](const V &self, double lhs) -> V {
          return V::All(lhs) - self;
        },
        doc.str().c_str(),
        pybind11::arg("lhs"));
  }

//TODO check if flaky warnings still need to be suppressed (switched to explicit lambda)
//#ifdef __clang__
//// Clang gives false warnings: https://bugs.llvm.org/show_bug.cgi?id=43124
//#pragma GCC diagnostic push
//#pragma GCC diagnostic ignored "-Wself-assign-overloaded"
//#endif  // __clang__
//  std::ostringstream().swap(doc);
//  doc << "Operator ``-= vec``.\n\nSubtracts the other :class:`~"
//      << module_name << '.' << VC_double::TypeName()
//      << "` values from this vector.";
//  vec_cls.def(pybind11::self -= pybind11::self, doc.str().c_str(), pybind11::arg("other"));
//#ifdef __clang__
//#pragma GCC diagnostic pop
//#endif  // __clang__
}


template<class V>
inline void RegisterVectorMultiplication(
    pybind11::class_<V> &vec,
    std::string_view module_name) {
  using value_type = typename V::value_type;
  using V_int32 = werkzeugkiste::geometry::Vec<value_type, V::ndim>;

  const std::string fqn_type = std::string(module_name) + "." + V::TypeName();
  const std::string fqn_int32 = std::string(module_name) + "." + V_int32::TypeName();


  // Vec * int
  std::ostringstream doc;
  doc << "Element-wise multiplication: :class:`~" << fqn_type
      << "` = :class:`~" << fqn_type << "` * :class:`int`.";
  vec.def(
      "__mul__",
      [](const V &self, int32_t rhs) -> V {
        // 32-bit integers can safely be cast into double precision types:
        return self * static_cast<value_type>(rhs);
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  // int * Vec
  std::ostringstream().swap(doc);
  doc << "Element-wise multiplication: :class:`~" << fqn_type
      << "` = :class:`int` * :class:`~" << fqn_type << "`.";
  vec.def(
      "__rmul__",
      [](const V &self, int32_t lhs) -> V {
        // 32-bit integers can safely be cast into double precision types:
        return self * static_cast<value_type>(lhs);
      },
      doc.str().c_str(),
      pybind11::arg("lhs"));


  // Vector * Vector (of the same type)
  std::ostringstream().swap(doc);
  doc << "Element-wise multiplication: :class:`~" << fqn_type
      << "` = :class:`~" << fqn_type << "` * :class:`~" << fqn_type << "`.";
  vec.def(
      "__mul__",
      [](const V &self, const V &rhs) -> V {
        return self * rhs;
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  if constexpr (std::is_floating_point_v<value_type>) {
    // Allow implicit upcasting of int32 vectors
    std::ostringstream().swap(doc);
    doc << "Element-wise multiplication: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_type
        << "` * :class:`~" << fqn_int32 << "`.";
    vec.def(
        "__mul__",
        [](const V &self, const V_int32 &rhs) -> V {
          V dbl = rhs.ToDouble();
          dbl *= self;
          return dbl;
        },
        doc.str().c_str(),
        pybind11::arg("rhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise multiplication: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_int32
        << "` * :class:`~" << fqn_type << "`.";
    vec.def(
        "__rmul__",
        [](const V &self, const V_int32 &lhs) -> V {
          V dbl = lhs.ToDouble();
          dbl *= self;
          return dbl;
        },
        doc.str().c_str(),
        pybind11::arg("lhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise multiplication: :class:`~" << fqn_type
        << "` = :class:`~" << fqn_type << "` * :class:`float`.";
    vec.def(
        "__mul__",
        [](const V &self, double rhs) -> V {
          return self * rhs;
        },
        doc.str().c_str(),
        pybind11::arg("rhs"));


    std::ostringstream().swap(doc);
    doc << "Element-wise multiplication: :class:`~" << fqn_type
        << "` = :class:`float` * :class:`~"
        << fqn_type << "`.";
    vec.def(
        "__rmul__",
        [](const V &self, double lhs) -> V {
          return self * lhs;
        },
        doc.str().c_str(),
        pybind11::arg("lhs"));
  }

  // Overloading `__imul__` is not needed
}


template<class V>
inline void RegisterVectorDivision(
    pybind11::class_<V> &vec,
    std::string_view module_name) {
  using value_type = typename V::value_type;
  using V_int32 = werkzeugkiste::geometry::Vec<value_type, V::ndim>;
  using V_double = werkzeugkiste::geometry::Vec<double, V::ndim>;

  const std::string fqn_type = std::string(module_name) + "." + V::TypeName();
  const std::string fqn_int32 = std::string(module_name) + "." + V_int32::TypeName();
  const std::string fqn_double = std::string(module_name) + "." + V_double::TypeName();

  std::string prefix{};
  if constexpr (std::is_integral_v<value_type>) {
    prefix = "Element-wise division with **implicit type conversion**. Returns\n"
       "a **double-precision** :class:`~" + fqn_double + "` = ";
  } else {
    prefix = "Element-wise division: :class:`~" + fqn_double + "` = ";
  }
  std::ostringstream doc;
  doc << prefix << ":class:`~" << fqn_type << "` / :class:`int`.";
  vec.def(
      "__truediv__",
      [](const V &self, int32_t rhs) -> V_double {
        // 32-bit integers can safely be cast into double precision types:
        V_double dbl = self.ToDouble();
        dbl /= static_cast<double>(rhs);
        return dbl;
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  std::ostringstream().swap(doc);
  doc << prefix << ":class:`~" << fqn_type << "` / :class:`float`.";
  vec.def(
      "__truediv__",
      [](const V &self, double rhs) -> V_double {
        V_double dbl = self.ToDouble();
        dbl /= rhs;
        return dbl;
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));

  std::ostringstream().swap(doc);
  doc << prefix << ":class:`~" << fqn_type << "` / :class:`~"
      << fqn_type << "`.";
  vec.def(
      "__truediv__",
      [](const V &self, const V &rhs) -> V_double {
        V_double dbl = self.ToDouble();
        dbl /= rhs.ToDouble();
        return dbl;
      },
      doc.str().c_str(),
      pybind11::arg("rhs"));


  //-----------------------------------------------------------
  // Inverse (scalar / vector)
  //TODO add to pytests
  std::ostringstream().swap(doc);
  doc << prefix << ":class:`int` / :class:`~" << fqn_type << "`.";
  vec.def(
      "__rtruediv__",
      [](const V &self, int32_t lhs) -> V_double {
        // 32-bit integers can safely be cast into double precision types:
        V_double all = V_double::All(static_cast<double>(lhs));
        all /= self.ToDouble();
        return all;
      },
      doc.str().c_str(),
      pybind11::arg("lhs"));

  std::ostringstream().swap(doc);
  doc << prefix << ":class:`float` / :class:`~" << fqn_type << "`.";
  vec.def(
      "__rtruediv__",
      [](const V &self, double lhs) -> V_double {
        V_double all = V_double::All(lhs);
        all /= self.ToDouble();
        return all;
      },
      doc.str().c_str(),
      pybind11::arg("lhs"));

  // Overloading `__idiv__` is not needed
}


template<typename Tp, std::size_t Dim>
void RegisterVector(pybind11::module &m) {
  static_assert(
      std::is_same_v<int32_t, Tp> || std::is_same_v<double, Tp>,
      "Vector bindings are only allowed for double precision floating point "
      "or int32_t type!");
  static_assert(Dim >= 2, "Vector must have at least 2 dimensions!");

  using VC = werkzeugkiste::geometry::Vec<Tp, Dim>;
  using VC_int32 = werkzeugkiste::geometry::Vec<int32_t, Dim>;
  using VC_double = werkzeugkiste::geometry::Vec<double, Dim>;
  const std::string module_name = m.attr("__name__").cast<std::string>();

  std::ostringstream doc;
  doc << "A " << Dim
      << "D vector of " << PythonVecTypeName<Tp>(true)
      << " coordinates.\n\n**Corresponding C++ API:** ``"
      << module_name << "::" << VC::TypeName() << "``.";

  pybind11::class_<VC> vec_cls(
        m, VC::TypeName().c_str(), pybind11::buffer_protocol(), doc.str().c_str());

  vec_cls.def(pybind11::init<>(), "Initializes all values to 0.");

  if constexpr (std::is_same_v<VC, VC_double>) {
    // 32-bit integers can be exactly represented by
    // a double. Thus, we allow implicit conversion.
    // Nice write-up: https://stackoverflow.com/a/43656339/400948
    std::ostringstream().swap(doc);
    doc << "Creates a double-precision vector from its 32-bit integral counterpart.";
    vec_cls.def(
        pybind11::init<>([](const VC_int32 &vec) {
            return vec.ToDouble();
          }),
        doc.str().c_str(),
        pybind11::arg("vec_int"));
    pybind11::implicitly_convertible<VC_int32, VC>();
  }

  std::ostringstream().swap(doc);
  doc << "Creates a vector where each coordinate is set to the given scalar."
         "\n\n**Corresponding C++ API:** ``" << module_name << "::"
      << VC::TypeName() << "::All``.";
  vec_cls.def_static(
        "all",
        &VC::All,
        doc.str().c_str(),
        pybind11::arg("value"));


  std::ostringstream().swap(doc);
  doc << "Creates a copy of the given ``" << VC::TypeName() << "``.";
  vec_cls.def(
        pybind11::init<const VC&>(), doc.str().c_str(), pybind11::arg("vec"));


  std::ostringstream().swap(doc);
  doc << "Creates a vector from a " << Dim << "-element :class:`tuple`.";
  vec_cls.def(
      pybind11::init<>(
        [module_name](const pybind11::tuple &tpl) {
          return VecFromTuple<Tp, Dim>(tpl, module_name);
        }),
      doc.str().c_str(),
      pybind11::arg("tpl"));


  std::ostringstream().swap(doc);
  doc << "Creates a vector from a " << Dim << "-element :class:`list`.";
  vec_cls.def(
        pybind11::init<>(
          [module_name](const pybind11::list &lst) {
            return VecFromList<Tp, Dim>(lst, module_name);
          }),
        doc.str().c_str(),
        pybind11::arg("lst"));


  std::ostringstream().swap(doc);
  doc << "Creates a vector from a " << Dim << "-element :class:`numpy.array`.";
  vec_cls.def(
        pybind11::init<>(
          [module_name](const pybind11::array &arr) {
            return VecFromArray<Tp, Dim>(arr, module_name);
          }),
        doc.str().c_str(),
        pybind11::arg("arr"));


  // Support the Python buffer protocol:
  vec_cls.def_buffer(&VecInfo<Tp, Dim>);


  // For convenience, also provide the dtype
  vec_cls.def_property_readonly(
        "dtype",
        [](const VC & /* self */) {
            return pybind11::dtype(
                  pybind11::format_descriptor<Tp>::format());
        },
        ":class:`numpy.dtype`: Underlying data type (read-only).");


  // ---------------------------------------
  // Explicit type conversion
  std::ostringstream().swap(doc);
  if constexpr(std::is_integral_v<Tp>) {
    doc << "Returns a copy of this vector as :class:`~"
        << module_name << '.' << VC_int32::TypeName()
        << "`\n\n**Corresponding C++ API:** ``" << module_name << "::"
        << VC::TypeName() << "::ToInteger``.";
  } else {
    doc << "Returns a :class:`~" << module_name << '.' << VC_int32::TypeName()
        << "` as the result\nof **explicitly casting/clipping** the values."
           "\n\n**Corresponding C++ API:** ``" << module_name << "::"
        << VC::TypeName() << "::ToInteger``.";
  }
  vec_cls.def(
        "as_int",
        &VC::ToInteger,
        doc.str().c_str());


  std::ostringstream().swap(doc);
  if constexpr(std::is_integral_v<Tp>) {
    doc << "Returns a :class:`~" << module_name << '.' << VC_int32::TypeName()
        << "` as the result\nof **explicitly casting** the values."
           "\n\n**Corresponding C++ API:** ``" << module_name << "::"
        << VC::TypeName() << "::ToDouble``.";
  } else {
    doc << "Returns a copy of this vector as :class:`~"
        << module_name << '.' << VC_double::TypeName()
        << "`\n\n**Corresponding C++ API:** ``" << module_name << "::"
        << VC::TypeName() << "::ToDouble``.";
  }
  vec_cls.def(
        "as_float",
        &VC::ToDouble,
        doc.str().c_str());


//TODO follow best practices: https://stackoverflow.com/a/2626364/400948
  vec_cls.def(
        "__repr__",
        [](const VC &v) {
          return "<"  + v.ToString(true) + ">";
        });

  //TODO pretty print
  vec_cls.def(
        "__str__",
        [](const VC &v) {
          return v.ToString(false);
        });


  std::ostringstream().swap(doc);
  doc << PythonVecTypeName<Tp>(false)
      << " Read-write access to the first dimension, *i.e.* same as ``vec[0]``.";
  vec_cls.def_property(
        "x",
        static_cast<const Tp &(VC::*)() const>(&VC::X),
        &VC::SetX,
        doc.str().c_str());


  if constexpr (Dim >= 2) {
    std::ostringstream().swap(doc);
    doc << PythonVecTypeName<Tp>(false)
        << " Read-write access to the second dimension, *i.e.* same as ``vec[1]``.";
    vec_cls.def_property(
        "y",
        [](const VC &self) -> Tp { return self.Y(); },
        [](VC &self, Tp value) -> void { self.SetY(value); },
        doc.str().c_str());
  }


  std::ostringstream().swap(doc);
  doc << "int: Number of dimensions (read-only), *i.e.* " << Dim << ".";
  vec_cls.def_property_readonly(
        "ndim",
        [](const VC&) { return Dim; },
        doc.str().c_str())
      .def(
        "__setitem__",
        [](VC &self, int index, Tp v) { self[index] = v; },
        "Allows accessing this vector's values via ``vec[idx]``.")
      .def(
        "__getitem__",
        [](const VC &self, int index) -> Tp { return self[index]; },
        "Allows accessing this vector's values via ``vec[i]``.");


  std::ostringstream().swap(doc);
  doc << "Returns a deep copy.\n\n"
         "**Corresponding C++ API:** Copy constructor of ``"
      << module_name << "::" << VC::TypeName() << "``.";
  vec_cls.def(
        "copy",
        [](const VC &self) { return VC(self); },
        doc.str().c_str());


  std::ostringstream().swap(doc);
  doc << "Returns :math:`\\max(v_i)`.\n\n**Corresponding C++ API:** ``"
      << module_name << "::" << VC::TypeName() << "::MaxValue``.";
  vec_cls.def("max_value", &VC::MaxValue, doc.str().c_str());


  std::ostringstream().swap(doc);
  doc << "Returns :math:`\\min(v_i)`.\n\n**Corresponding C++ API:** ``"
      << module_name << "::" << VC::TypeName() << "::MinValue``.";
  vec_cls.def("min_value", &VC::MinValue, doc.str().c_str());


  std::ostringstream().swap(doc);
  doc << "Returns :math:`i = \\arg_i \\max(v_i)`.\n\n**Corresponding C++ API:** ``"
      << module_name << "::" << VC::TypeName() << "::MaxIndex``.";
  vec_cls.def("max_index", &VC::MaxIndex, doc.str().c_str());


  std::ostringstream().swap(doc);
  doc << "Returns :math:`i = \\arg_i \\min(v_i)`.\n\n**Corresponding C++ API:** ``"
      << module_name << "::" << VC::TypeName() << "::MinIndex``.";
  vec_cls.def("min_index", &VC::MinIndex, doc.str().c_str());


  std::ostringstream().swap(doc);
  doc << "Returns the length of this vector.\n\n**Corresponding C++ API:** ``"
      << module_name << "::" << VC::TypeName() << "::Length``.";
  vec_cls.def("length", &VC::Length, doc.str().c_str());


  std::ostringstream().swap(doc);
  doc << "Returns the squared length of this vector.\n\n**Corresponding C++ API:** ``"
      << module_name << "::" << VC::TypeName() << "::LengthSquared``.";
  vec_cls.def("length_squared", &VC::LengthSquared, "Returns this vector's squared length.");


  std::ostringstream().swap(doc);
  doc << "Returns the dot product of ``self`` and the other :class:`~"
      << module_name << '.' << VC::TypeName() << "`.";
  vec_cls.def("dot", &VC::Dot, doc.str().c_str(), pybind11::arg("other"));


  std::ostringstream().swap(doc);
  doc << "Returns the Euclidean distance between ``self`` and the other :class:`~"
      << module_name << '.' << VC::TypeName() << "`.";
  vec_cls.def("distance_l2", &VC::DistanceEuclidean, doc.str().c_str(),
           pybind11::arg("other"));


  std::ostringstream().swap(doc);
  doc << "Returns the Manhattan distance between ``self`` and the other :class:`~"
      << module_name << '.' << VC::TypeName() << "`.";
  vec_cls.def("distance_l1", &VC::DistanceManhattan, doc.str().c_str(),
           pybind11::arg("other"));


  std::ostringstream().swap(doc);
  doc << "Returns the :class:`~" <<  module_name << '.' << VC::TypeName()
      << "` direction vector, ``other - self``.";
  vec_cls.def("direction_vector", &VC::DirectionVector,
           doc.str().c_str(), pybind11::arg("other"));


  std::ostringstream().swap(doc);
  doc << "Returns the corresponding unit vector as :class:`~"
      << module_name << '.' << VC_double::TypeName() << "`.";
  vec_cls.def("unit_vector", &VC::UnitVector, doc.str().c_str());


  std::ostringstream().swap(doc);
  doc << ":class:`~" << module_name << '.' << VC_double::TypeName()
      << "` instances can be pickled.";
  vec_cls.def(
        pybind11::pickle(
          &VecToTuple<Tp, Dim>,
          [module_name](const pybind11::tuple &tpl) {
            return VecFromTuple<Tp, Dim>(tpl, module_name);
          }),
        doc.str().c_str());


  if constexpr (std::is_integral_v<Tp>) {
    vec_cls.def(
        "__eq__",
        [](const VC &a, const VC &b) -> bool { return a == b; },
        "Checks for equality.",
        pybind11::arg("other"));

    vec_cls.def(
        "__ne__",
        [](const VC &a, const VC &b) -> bool { return a != b; },
        "Checks for inequality.",
        pybind11::arg("other"));
  } else {
    std::ostringstream().swap(doc);
    doc << "Returns ``True`` if the two vectors are *approximately equal*.\n\n"
           "Uses a combined test with both relative and absolute tolerance.\n"
           "Modulo special cases (NaN, infinity), two vectors\n"
           ":math:`x = (x_i)_{i=1}^" << Dim
        << "` and :math:`y = (y_i)_{i=1}^" << Dim
        <<  "` are considered\n*approximately equal* if for all their values\n\n"
           ":math:`|x_i - y_i| \\leq \\epsilon_{\\text{rel}} * \\max(|x_i|, |y_i|) "
           "\\quad\\text{or}\\quad |x_i - y_i| \\leq \\epsilon_{\\text{abs}}`,\n\nwhere "
           ":math:`\\epsilon_{\\text{rel}} = 1e-9` and "
           ":math:`\\epsilon_{\\text{abs}} = 1e-12`."
           "\n\n**Corresponding C++ API:** ``" << module_name << "::"
        << VC::TypeName() << "::operator==`` and ``::IsClose``";
    vec_cls.def(
        "__eq__",
        [](const VC &a, const VC &b) -> bool {
          return a.IsClose(b, 1e-9, 1e-12);
        },
        doc.str().c_str(),
        pybind11::arg("other"));

    doc << "Returns ``True`` if the two vectors differ *sufficiently*.\n\n"
           "As this operator computes ``!(vec1 == vec2)``, refer to\n"
           ":meth:`~" << module_name << '.' << VC::TypeName() << ".__eq__` for details.";
    vec_cls.def(
        "__ne__",
        [](const VC &a, const VC &b) -> bool { return a != b; },
        doc.str().c_str(),
        pybind11::arg("other"));
  }


  std::ostringstream().swap(doc);
  doc << "Returns a :class:`~" << module_name << '.' << VC_double::TypeName()
      << "`, where all values are negated.";
  vec_cls.def(-pybind11::self, doc.str().c_str());


  std::ostringstream().swap(doc);
  doc << "Returns a :class:`~" << module_name << '.' << VC_double::TypeName()
      << "`, where all values\nare replaced by their absolute counterparts.";
  vec_cls.def(
      "__abs__",
      &VC::Absolute,
      doc.str().c_str());

  RegisterVectorAdditionSubtraction(vec_cls, module_name);
  RegisterVectorMultiplication(vec_cls, module_name);
  RegisterVectorDivision(vec_cls, module_name);


  // Specific for 2-dim vectors
  if constexpr (Dim == 2) {
    std::ostringstream().swap(doc);
    doc << "Explicit initialization from " << Dim << " separate values.";
    vec_cls.def(
          pybind11::init<Tp, Tp>(),
          doc.str().c_str(),
          pybind11::arg("x"), pybind11::arg("y"));

    std::ostringstream().swap(doc);
    doc << PythonVecTypeName<Tp>(false)
        << " Provides alternative read-write access to\n"
           "  the first dimension, (*i.e.* :attr:`x`).\n\n"
           "  Can be useful if you want to use this 2D vector to\n"
           "  represent a 2D *size*. This property is only available\n"
           "  for 2-dimensional vectors.";
    vec_cls.def_property(
          "width",
          [](const VC &self) -> Tp { return self.Width(); },
          [](VC &self, Tp value) -> void { self.SetWidth(value); },
          doc.str().c_str());


    std::ostringstream().swap(doc);
    doc << PythonVecTypeName<Tp>(false)
        << " Provides alternative read-write access to\n"
           "  the second dimension, (*i.e.* :attr:`y`).\n\n"
           "  Can be useful if you want to use this 2D vector to\n"
           "  represent a 2D *size*. This property is only available\n"
           "  for 2-dimensional vectors.";
    vec_cls.def_property(
          "height",
          [](const VC &self) -> Tp { return self.Height(); },
          [](VC &self, Tp value) -> void { self.SetHeight(value); },
          doc.str().c_str());

    //TODO add 2d specialties (perpendicular, rotate, etc)
  }

  // Specific for 3-dim vectors
  if constexpr (Dim == 3) {
    std::ostringstream().swap(doc);
    doc << "Explicit initialization from " << Dim << " separate values.";
    vec_cls.def(
          pybind11::init<Tp, Tp, Tp>(),
          doc.str().c_str(),
          pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z"));

    vec_cls.def(
          "cross",
          [](const VC &self, const VC &other) { return self.Cross(other); },
          "Computes the cross product.",
          pybind11::arg("other"));

    //TODO add 3d specialties
  }


  if constexpr (Dim > 2) {
    std::ostringstream().swap(doc);
    doc << PythonVecTypeName<Tp>(false)
        << " Read-write access to the third dimension, *i.e.* same as ``vec[2]``.";
    vec_cls.def_property(
          "z",
          [](const VC &self) -> Tp { return self.Z(); },
          [](VC &self, Tp value) -> void { self.SetZ(value); },
          doc.str().c_str());
  }


  // Specific for 4-and-more-dim vectors
  if constexpr (Dim > 3) {
    if constexpr (Dim == 4) {
      std::ostringstream().swap(doc);
      doc << "Explicit initialization from " << Dim << " separate values.";
      vec_cls.def(
            pybind11::init<Tp, Tp, Tp, Tp>(),
            doc.str().c_str(),
            pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("z"), pybind11::arg("w"));
    }


    std::ostringstream().swap(doc);
    doc << PythonVecTypeName<Tp>(false)
        << " Read-write access to the fourth dimension, "
           "*i.e.*  same as ``vec[3]``.";
    vec_cls.def_property(
          "w",
          static_cast<const Tp &(VC::*)() const>(&VC::W),
          &VC::SetW,
          doc.str().c_str());
  }

  pybind11::implicitly_convertible<pybind11::tuple, VC>();
  pybind11::implicitly_convertible<pybind11::list, VC>();
  pybind11::implicitly_convertible<pybind11::array, VC>();
}

} // namespace werkzeugkiste::bindings


