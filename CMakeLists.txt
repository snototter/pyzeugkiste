cmake_minimum_required(VERSION 3.14)

# ##############################################################################
# In-source build guard
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
    FATAL_ERROR
      "In-source builds are not supported. "
      "Please read the BUILDING document before trying to build this project. "
      "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first.")
endif()

# ##############################################################################
# Set up the project

# Had to replace the simple project() call by the following workaround, because
# the macOS builds failed when setting the version. Exemplary failed build:
# https://github.com/snototter/pyzeugkiste/actions/runs/4098116828/jobs/7066986892
# On linux, setting the version via the scikit-build-core provided variable
# worked as expected. We just need to catch the "development case", i.e. when
# neither pyzeugkiste_VERSION, nor the scikit SKBUILD_PROJECT_VERSION are
# provided. (Calling project() with an empty version string would result in a
# CMake warning I want to suppress with the following)
if(NOT pyzeugkiste_VERSION)
  string(REGEX REPLACE "^.*([0-9]+\\.[0-9]+\\.[0-9]+).*$" "\\1"
                       pyzeugkiste_VERSION "${SKBUILD_PROJECT_VERSION}")
endif()

# If we don't have a version string, we omit the VERSION keyword to avoid the
# CMake warning. If we *do* have a valid version string, we do *not* set it on
# macOS (or else the build failed).
set(pyzeugkiste_DESCRIPTION
    "PyZeugKiste: Python bindings for werkzeugkiste and additional python-only helpers."
)
set(pyzeugkiste_URL "https://pyzeugkiste.readthedocs.io/en/latest/")

if(APPLE OR (NOT pyzeugkiste_VERSION))
  project(
    pyzeugkiste
    DESCRIPTION ${pyzeugkiste_DESCRIPTION}
    HOMEPAGE_URL ${pyzeugkiste_URL}
    LANGUAGES CXX)
else()
  project(
    pyzeugkiste
    VERSION ${pyzeugkiste_VERSION}
    DESCRIPTION ${pyzeugkiste_DESCRIPTION}
    HOMEPAGE_URL ${pyzeugkiste_URL}
    LANGUAGES CXX)
endif()

if(NOT SKBUILD_PROJECT_NAME)
  set(SKBUILD_PROJECT_NAME pyzeugkiste)
endif()
set(pyzeugkiste_BINDINGS_TARGET _core)
set(pyzeugkiste_BINDINGS_PRINT_NAME ${SKBUILD_PROJECT_NAME})

# Header files
set(pyzeugkiste_HEADER_FILES
    include/werkzeugkiste-bindings/vector_bindings.h
    include/werkzeugkiste-bindings/line2d_bindings.h
    include/werkzeugkiste-bindings/config_bindings.h)

# Source files
set(pyzeugkiste_SOURCE_FILES src/pyzeugkiste_core.cpp)

# ##############################################################################
# Ensure pybind11 is available

# We use FetchContent to configure all dependencies
include(FetchContent)

# If built outside scikit-build-core (i.e. during development), pybind11 may not
# be available.
find_package(pybind11 CONFIG QUIET)
if(NOT pybind11_FOUND)
  FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11.git
    GIT_TAG v2.10.3)
  FetchContent_MakeAvailable(pybind11)
endif()
message(STATUS "Using pybind11 ${pybind11_VERSION}")

# ##############################################################################
# Register the pyzeugkiste library

# pybind11 provides add_module which is basically a wrapper around CMake's
# add_library
pybind11_add_module(${pyzeugkiste_BINDINGS_TARGET} MODULE
                    ${pyzeugkiste_HEADER_FILES} ${pyzeugkiste_SOURCE_FILES})

# ##############################################################################
# Ensure werkzeugkiste is available
FetchContent_Declare(
  werkzeugkiste
  GIT_REPOSITORY https://github.com/snototter/werkzeugkiste.git
  GIT_TAG main)
FetchContent_MakeAvailable(werkzeugkiste)
message(STATUS "Using werkzeugkiste ${werkzeugkiste_VERSION}")

# Components of werkzeugkiste are compiled with position independent code by
# default. Thus, we don't need to explicitly enable PIC here.
target_link_libraries(${pyzeugkiste_BINDINGS_TARGET}
                      PUBLIC werkzeugkiste::geometry)
target_link_libraries(
  ${pyzeugkiste_BINDINGS_TARGET}
  PRIVATE werkzeugkiste::config werkzeugkiste::logging werkzeugkiste::strings)

# ##############################################################################
# Set
set_target_properties(
  ${pyzeugkiste_BINDINGS_TARGET}
  PROPERTIES CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES
             VERSION "${PROJECT_VERSION}"
             SOVERSION "${PROJECT_VERSION_MAJOR}"
             LINKER_LANGUAGE CXX)

target_compile_features(${pyzeugkiste_BINDINGS_TARGET} PUBLIC cxx_std_17)

# Once 'werkzeugkiste::config' has been added to the target, the linker broke.
# Older versions (Ubuntu 18) complained about non-PIC code (although it was
# enabled on all(!) targets); newer versions (Ubuntu 20) crashed with: "lto1:
# internal compiler error: in add_symbol_to_partition_1, at
# lto/lto-partition.c:153" which lead me to this bug report:
# https://gcc.gnu.org/bugzilla/show_bug.cgi?id=88550 After disabling LTO, this
# issue persists. --> all builds still used -fPIC -fLTO -shared and caused the
# mentioned errors... The "interesting" part is that the pip installation works
# without any issue whatsoever (would be interesting to dump the full CMake
# environment on the different pip environments, both locally and on the CI
# runners...) set_property( TARGET ${pyzeugkiste_BINDINGS_TARGET} PROPERTY
# INTERPROCEDURAL_OPTIMIZATION FALSE)
#
# Other observations: * Only linking to wzk::config (but not wzk::strings)
# yields the same issue. * Removing wzk::config works, so this issue is
# definitely caused by "something" in it :-/

target_include_directories(
  ${pyzeugkiste_BINDINGS_TARGET}
  PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

target_compile_definitions(
  ${pyzeugkiste_BINDINGS_TARGET}
  PRIVATE pyzeugkiste_PYMODULE_IDENTIFIER=${pyzeugkiste_BINDINGS_TARGET}
          pyzeugkiste_PYMODULE_PRINT_NAME=${pyzeugkiste_BINDINGS_PRINT_NAME})

if(pyzeugkiste_VERSION)
  target_compile_definitions(
    ${pyzeugkiste_BINDINGS_TARGET}
    PRIVATE pyzeugkiste_VERSION_INFO=${pyzeugkiste_VERSION})
endif()

set_target_properties(
  ${pyzeugkiste_BINDINGS_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON
                                            CXX_STANDARD_REQUIRED ON)

# TODO check if SKBUILD_SOABI has to be set, see example at
# https://pypi.org/project/scikit-build-core/

# TODO add custom test/coverage/lint target?

install(TARGETS ${pyzeugkiste_BINDINGS_TARGET}
        DESTINATION ${SKBUILD_PROJECT_NAME})

message(STATUS "Configured pyzeugkiste ${pyzeugkiste_VERSION}")
