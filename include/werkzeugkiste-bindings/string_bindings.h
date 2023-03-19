#ifndef WERKZEUGKISTE_BINDINGS_STRINGS_H
#define WERKZEUGKISTE_BINDINGS_STRINGS_H

#include <pybind11/pybind11.h>

#include <werkzeugkiste/strings/strings.h>

namespace werkzeugkiste::bindings {
inline void RegisterStringUtils(pybind11::module &main_module) {
  pybind11::module m = main_module.def_submodule("_str");
  m.doc() = R"doc(
    String utilities.

    TODO summary
    )doc";

  std::string doc{R"doc(
    Returns the Levenshtein (string edit) distance.
    
    Args:
      s1: First string.
      s2: Second string.
      
    Returns:
      The minimum number of single-character edits (i.e. insertions,
      deletions or substitutions) required to change one string into the other.
    )doc"};
  m.def("levenshtein_distance",
        &werkzeugkiste::strings::LevenshteinDistance,
        doc.c_str(), pybind11::arg("s1"), pybind11::arg("s2"));

  doc = R"doc(
    Clips the string if it exceeds the given length.
    
    Args:
      string: The string.
      length: The desired maximum length.
      ellipsis: The ellipsis to be inserted where the string has been clipped.
      pos: Where to clip the string & place the ellipsis: *Left* if ``< 0``,
        *centered* if ``0``, *right* if ``> 0``.
    )doc";
  m.def("shorten",
        &werkzeugkiste::strings::Shorten,
        doc.c_str(), pybind11::arg("string"), pybind11::arg("length"),
        pybind11::arg("pos") = -1, pybind11::arg("ellipsis") = "...");

  doc = R"doc(
    Returns a slug representation of the string.

    TODO document all replacements and conversions.

    Args:
      string: The string to be slugified.
      strip_dashes: If ``True``, the slug will contain no dashes.
    )doc";
  m.def("slugify",
        &werkzeugkiste::strings::Slug,
        doc.c_str(), pybind11::arg("string"),
        pybind11::arg("strip_dashes") = false);

  // TODO add bindings for ClipURL, GetURLProtocol, ObscureURLAuthentication
}
}  // namespace werkzeugkiste::bindings

#endif // WERKZEUGKISTE_BINDINGS_STRINGS_H
