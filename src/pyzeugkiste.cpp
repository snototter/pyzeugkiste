#include <pybind11/pybind11.h>

#include <werkzeugkiste-bindings/vector_bindings.h>
//#include <werkzeugkiste-bindings/line2d.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

///----------------------------------------------------------------------------
/// Module definition
PYBIND11_MODULE(pyzeugkiste_PYMODULE_NAME, m) {
  m.doc() = R"doc(
    TODO doc
    )doc";

  //-------------------------------------------------
  // Primitives
  //
  // Vectors need to be registered first because they
  // are used by several other geometric primitives.
  werkzeugkiste::bindings::RegisterVector<double, 2>(m);
  werkzeugkiste::bindings::RegisterVector<double, 3>(m);
  werkzeugkiste::bindings::RegisterVector<int32_t, 2>(m);
  werkzeugkiste::bindings::RegisterVector<int32_t, 3>(m);

//  werkzeugkiste::bindings::RegisterLine2d(m);

#ifdef pyzeugkiste_VERSION_INFO
    m.attr("__version__") = STRINGIFY(pyzeugkiste_VERSION_INFO);
#else
    m.attr("__version__") = "dev";
#endif
}
