cmake_minimum_required(VERSION 3.14)

###############################################################################
# In-source build guard
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
  message(
      FATAL_ERROR
      "In-source builds are not supported. "
      "Please read the BUILDING document before trying to build this project. "
      "You may need to delete 'CMakeCache.txt' and 'CMakeFiles/' first."
  )
endif()


#########################################################################################
# ---- Set up the project ----

# This build's version could have already been set, e.g. by the CMakeBuild
# extension in `setup.py`
if(NOT pyzeugkiste_VERSION)
  file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" pyzeugkiste_VERSION)
endif()


# Set up the project
project(pyzeugkiste VERSION ${pyzeugkiste_VERSION} LANGUAGES CXX)
project(
    pyzeugkiste
    VERSION ${pyzeugkiste_VERSION}
    DESCRIPTION "Python bindings for werkzeugkiste and additional helpers."
    HOMEPAGE_URL "https://github.com/snototter/pyzeugkiste"
    LANGUAGES CXX
)

set(pyzeugkiste_BINDINGS_TARGET _core)
set(pyzeugkiste_BINDINGS_PRINT_NAME pyzeugkiste)



# Header files
set(pyzeugkiste_HEADER_FILES
    include/werkzeugkiste-bindings/logging.h
    include/werkzeugkiste-bindings/vector_bindings.h
)

# Source files
set(pyzeugkiste_SOURCE_FILES
    src/bindings_core.cpp
)


# pybind11 provides add_module which is basically a wrapper of CMake's add_library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/pybind11)


pybind11_add_module(
    ${pyzeugkiste_BINDINGS_TARGET}
    ${pyzeugkiste_HEADER_FILES}
    ${pyzeugkiste_SOURCE_FILES}
)


set_target_properties(
    ${pyzeugkiste_BINDINGS_TARGET}
    PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
    LINKER_LANGUAGE CXX
)

target_compile_features(
    ${pyzeugkiste_BINDINGS_TARGET}
    PUBLIC cxx_std_17
)

target_include_directories(
    ${pyzeugkiste_BINDINGS_TARGET}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)


#target_include_directories(
#    ${pyzeugkiste_BINDINGS_TARGET} SYSTEM
#    PUBLIC
#        "${CMAKE_CURRENT_SOURCE_DIR}/include"
#)


# Fetch the library:
include(FetchContent)
FetchContent_Declare(
    werkzeugkiste
    GIT_REPOSITORY https://github.com/snototter/werkzeugkiste.git
    GIT_TAG main)
FetchContent_MakeAvailable(werkzeugkiste)
message(STATUS "werkzeugkiste v${werkzeugkiste_VERSION}")

# We link them statically, but need to ensure that the components are
# build with position independent code.
set_target_properties(
    werkzeugkiste-geometry
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(
    ${pyzeugkiste_BINDINGS_TARGET}
    PUBLIC
    werkzeugkiste::geometry
)


target_compile_definitions(
    ${pyzeugkiste_BINDINGS_TARGET}
    PRIVATE
    pyzeugkiste_VERSION_INFO=${pyzeugkiste_VERSION}
    pyzeugkiste_PYMODULE_IDENTIFIER=${pyzeugkiste_BINDINGS_TARGET}
    pyzeugkiste_PYMODULE_PRINT_NAME=${pyzeugkiste_BINDINGS_PRINT_NAME}
)


set_target_properties(
    ${pyzeugkiste_BINDINGS_TARGET}
    PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)



#    target_include_directories(${viren2d_TARGET_PYTHON_LIB} PRIVATE
#        "${CMAKE_CURRENT_SOURCE_DIR}/src")

#    set_target_properties(${viren2d_TARGET_PYTHON_LIB} PROPERTIES
#        OUTPUT_NAME ${PROJECT_NAME})

#    target_link_libraries(${viren2d_TARGET_PYTHON_LIB}
#        PRIVATE ${viren2d_TARGET_CPP_LIB}::${viren2d_TARGET_CPP_LIB})
#endif()


#TODO add custom test/coverage/lint target?

install(TARGETS ${pyzeugkiste_BINDINGS_TARGET} DESTINATION pyzeugkiste)

message(STATUS "Configured pyzeugkiste v${pyzeugkiste_VERSION}")

