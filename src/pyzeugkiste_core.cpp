#include <pybind11/pybind11.h>
#include <werkzeugkiste-bindings/config_bindings.h>
#include <werkzeugkiste-bindings/line2d_bindings.h>
#include <werkzeugkiste-bindings/vector_bindings.h>
#include <werkzeugkiste-bindings/string_bindings.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

void RegisterGeometryUtils(pybind11::module &m) {
  pybind11::module geo = m.def_submodule("_geo");
  geo.doc() = R"doc(
    Math utils for 2D/3D geometry.

    TODO summary
    )doc";

  // const std::string print_name =
  //     std::string{MACRO_STRINGIFY(pyzeugkiste_PYMODULE_PRINT_NAME)}
  //     + ".geo";

  //-------------------------------------------------
  // Primitives
  //
  // Vectors need to be registered first because they
  // are used by several other geometric primitives.
  //  const std::string main_module_name{
  //  MACRO_STRINGIFY(pyzeugkiste_PYMODULE_PRINT_NAME) }; TODO remove compile
  //  definition const std::string geo_module_name{ main_module_name +
  //  ".geometry" };
  werkzeugkiste::bindings::RegisterVector<double, 2>(geo);
  werkzeugkiste::bindings::RegisterVector<double, 3>(geo);
  werkzeugkiste::bindings::RegisterVector<int32_t, 2>(geo);
  werkzeugkiste::bindings::RegisterVector<int32_t, 3>(geo);

  werkzeugkiste::bindings::RegisterLine2d(geo);
}

///----------------------------------------------------------------------------
/// Module definition
PYBIND11_MODULE(pyzeugkiste_PYMODULE_IDENTIFIER, m) {
  m.doc() = R"doc(
    PyZeugKiste: A Python utility collection

    Submodules:
       * `args`
       * `config`
       * `files`
       * `geo`
       * `strings`
    )doc";

  RegisterGeometryUtils(m);
  werkzeugkiste::bindings::RegisterConfigUtils(m);
  werkzeugkiste::bindings::RegisterStringUtils(m);

#ifdef pyzeugkiste_VERSION_INFO
  m.attr("__version__") = MACRO_STRINGIFY(pyzeugkiste_VERSION_INFO);
#else
  m.attr("__version__") = "dev";
#endif
}
