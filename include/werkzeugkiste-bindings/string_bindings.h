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

/*
/// Clips the given URL string to include only the
/// protocol and domain, *i.e.* server paths & parameters
/// will be excluded.
WERKZEUGKISTE_STRINGS_EXPORT
std::string ClipUrl(const std::string& url);

/// Sets `protocol` to the URL's protocol, e.g.
/// `https://`, `rtp://`, ...
/// Returns true if the `url` string contained a
/// protocol part.
WERKZEUGKISTE_STRINGS_EXPORT
bool GetUrlProtocol(const std::string& url,
                    std::string& protocol,    // NOLINT
                    std::string& remainder);  // NOLINT

/// Returns the URL after replacing any plaintext
/// authentication data by the text `<auth>`.
WERKZEUGKISTE_STRINGS_EXPORT
std::string ObscureUrlAuthentication(const std::string& url);

/// Returns a copy where all given characters have been removed.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Remove(std::string_view s, std::initializer_list<char> chars);

/// Returns a copy where the given character has been removed.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Remove(std::string_view s, char c);

/// Returns a slug representation of the string.
///
/// The input will be converted to lower case & trimmed.
/// The number sign/hash will be replaced by "nr". Any
/// other non-alphanumeric symbols will be replaced by
/// dashes.
/// If `strip_dashes` is true, the remaining dashes will
/// then also be stripped: e.g. ` img_dir` would
/// become `imgdir`.
WERKZEUGKISTE_STRINGS_EXPORT
std::string Slug(std::string_view s, bool strip_dashes = false);

/// Returns a string with length <= `desired_length`,
/// where the customizable `ellipsis` has been inserted
/// to indicate that the input string has been clipped.
///
/// Argument ellipsis_position specifies where the ellipsis
/// will be placed:
/// * `< 0`: Left
/// * `0`: Centered
/// * `> 0`: Right
WERKZEUGKISTE_STRINGS_EXPORT
std::string Shorten(std::string_view s, std::size_t desired_length,
                    int ellipsis_position = -1,
                    std::string_view ellipsis = "...");
*/
}
}  // namespace werkzeugkiste::bindings

#endif // WERKZEUGKISTE_BINDINGS_STRINGS_H
