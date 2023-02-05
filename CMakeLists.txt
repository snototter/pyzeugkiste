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
# ---- Set up the project ----

# This build's version could have already been set, e.g. by the CMakeBuild
# extension in `setup.py` if(NOT pyzeugkiste_VERSION) file(STRINGS
# "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" pyzeugkiste_VERSION)
# set(pyzeugkiste_VERSION "dev") endif() Cannot set it to "dev", because this
# would break the CMake "project()" definition, which requires a
# semver-compatible version.

if(NOT pyzeugkiste_VERSION)
  set(pyzeugkiste_VERSION ${SKBUILD_PROJECT_VERSION})
endif()

# Set up the project project(pyzeugkiste VERSION ${pyzeugkiste_VERSION}
# LANGUAGES CXX)
project(
  pyzeugkiste
  VERSION ${pyzeugkiste_VERSION}
  DESCRIPTION
    "PyZeugKiste: Python bindings for werkzeugkiste and additional python-only helpers."
  HOMEPAGE_URL "https://github.com/snototter/pyzeugkiste"
  LANGUAGES CXX)

set(pyzeugkiste_BINDINGS_TARGET _core)
set(pyzeugkiste_BINDINGS_PRINT_NAME ${SKBUILD_PROJECT_NAME})

# Header files
set(pyzeugkiste_HEADER_FILES
    include/werkzeugkiste-bindings/vector_bindings.h
    include/werkzeugkiste-bindings/line2d_bindings.h
    include/werkzeugkiste-bindings/config_bindings.h)

# Source files
set(pyzeugkiste_SOURCE_FILES src/pyzeugkiste_core.cpp)

# pybind11 provides add_module which is basically a wrapper of CMake's
# add_library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/pybind11)

pybind11_add_module(${pyzeugkiste_BINDINGS_TARGET} MODULE
                    ${pyzeugkiste_HEADER_FILES} ${pyzeugkiste_SOURCE_FILES})

set_target_properties(
  ${pyzeugkiste_BINDINGS_TARGET}
  PROPERTIES CXX_VISIBILITY_PRESET hidden
             VISIBILITY_INLINES_HIDDEN YES
             VERSION "${PROJECT_VERSION}"
             SOVERSION "${PROJECT_VERSION_MAJOR}"
             LINKER_LANGUAGE CXX)

target_compile_features(${pyzeugkiste_BINDINGS_TARGET} PUBLIC cxx_std_17)

target_include_directories(
  ${pyzeugkiste_BINDINGS_TARGET}
  PUBLIC "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>")

# target_include_directories( ${pyzeugkiste_BINDINGS_TARGET} SYSTEM PUBLIC
# "${CMAKE_CURRENT_SOURCE_DIR}/include" )

# Fetch the library:
include(FetchContent)
FetchContent_Declare(
  werkzeugkiste
  GIT_REPOSITORY https://github.com/snototter/werkzeugkiste.git
  GIT_TAG main)
FetchContent_MakeAvailable(werkzeugkiste)
message(STATUS "werkzeugkiste v${werkzeugkiste_VERSION}")

# We link the werkzeugkiste components statically, but need to ensure that they
# are build with position independent code. Otherwise, the python binding, which
# is a shared library, can't be linked.
target_link_libraries(${pyzeugkiste_BINDINGS_TARGET}
                      PUBLIC werkzeugkiste::geometry)
target_link_libraries(
  ${pyzeugkiste_BINDINGS_TARGET} PRIVATE werkzeugkiste::config
                                         werkzeugkiste::logging
  # werkzeugkiste::strings
)

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

# target_include_directories(${viren2d_TARGET_PYTHON_LIB} PRIVATE
# "${CMAKE_CURRENT_SOURCE_DIR}/src")

# set_target_properties(${viren2d_TARGET_PYTHON_LIB} PROPERTIES OUTPUT_NAME
# ${PROJECT_NAME})

# target_link_libraries(${viren2d_TARGET_PYTHON_LIB} PRIVATE
# ${viren2d_TARGET_CPP_LIB}::${viren2d_TARGET_CPP_LIB}) endif()

# TODO add custom test/coverage/lint target?

install(TARGETS ${pyzeugkiste_BINDINGS_TARGET}
        DESTINATION ${SKBUILD_PROJECT_NAME})

message(STATUS "Configured pyzeugkiste v${pyzeugkiste_VERSION}")
