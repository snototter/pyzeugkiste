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

set(pyzeugkiste_PYMODULE_NAME _pyzeugkiste)


# Set default build type if not specified
#if(NOT CMAKE_BUILD_TYPE)
#  set(CMAKE_BUILD_TYPE Release)
#endif()

#FIXME
#set(CMAKE_CXX_STANDARD 17)  # We use std::exchange
#set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)  # Only std c++17, not gnu++17
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra") # TODO(cmake-improvement) gcc vs msvc vs... currently assumes linux, check CMakeLists of werkzeugkiste or spdlog!
#set(CMAKE_CXX_FLAGS_DEBUG "-g")
#set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# Compile library with -fPIC
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

###############################################################################
# Library
###############################################################################


# Header files
set(pyzeugkiste_HEADER_FILES
    include/werkzeugkiste-bindings/logging.h
    include/werkzeugkiste-bindings/vector_bindings.h

#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/werkzeugkiste/include/werkzeugkiste/geometry/vector.h
#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/werkzeugkiste/include/werkzeugkiste/geometry/primitives.h
)

# Source files
set(pyzeugkiste_SOURCE_FILES
    src/pyzeugkiste.cpp

#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/werkzeugkiste/src/geometry/vector.cpp    
#    ${CMAKE_CURRENT_SOURCE_DIR}/libs/werkzeugkiste/src/geometry/primitives.cpp
)



#add_library(${viren2d_TARGET_CPP_LIB}
#    ${viren2d_PUBLIC_HEADER_FILES}
#    ${viren2d_PRIVATE_HEADER_FILES}
#    ${viren2d_SOURCE_FILES})

# Add a namespace'd alias
#add_library(${viren2d_TARGET_CPP_LIB}::${viren2d_TARGET_CPP_LIB} ALIAS ${viren2d_TARGET_CPP_LIB})

#set_target_properties(${viren2d_TARGET_CPP_LIB}
#    PROPERTIES
#    PUBLIC_HEADER "${viren2d_PUBLIC_HEADER_FILES}"
#    DEBUG_POSTFIX "d")


#target_include_directories(${viren2d_TARGET_CPP_LIB}
#    PRIVATE
#        "${CMAKE_CURRENT_SOURCE_DIR}/src"
#        "${CMAKE_CURRENT_SOURCE_DIR}/libs/stb"
#    PUBLIC
#        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#        $<INSTALL_INTERFACE:${INSTALL_DIR_INCLUDE}>)


# pybind11 provides add_module which is basically a wrapper of CMake's add_library
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libs/pybind11)


pybind11_add_module(${pyzeugkiste_PYMODULE_NAME}
    ${pyzeugkiste_HEADER_FILES}
    ${pyzeugkiste_SOURCE_FILES}
)


set_target_properties(
    ${pyzeugkiste_PYMODULE_NAME} PROPERTIES
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN YES
    VERSION "${PROJECT_VERSION}"
    SOVERSION "${PROJECT_VERSION_MAJOR}"
#    EXPORT_NAME ${pyzeugkiste_PYMODULE_NAME}
#    OUTPUT_NAME ${pyzeugkiste_PYMODULE_NAME}
    LINKER_LANGUAGE CXX
)

target_compile_features(${pyzeugkiste_PYMODULE_NAME}
    PUBLIC cxx_std_17
)

target_include_directories(
    ${pyzeugkiste_PYMODULE_NAME}
    PUBLIC
    "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
)


target_include_directories(
    ${pyzeugkiste_PYMODULE_NAME} SYSTEM
    PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
)


#add_subdirectory(${PROJECT_SOURCE_DIR}/libs/werkzeugkiste)
# Override settings to avoid linking errors
#set_target_properties(
#    werkzeugkiste-geometry
#    PROPERTIES
#        POSITION_INDEPENDENT_CODE ON
#)
#
#target_compile_definitions(
#    werkzeugkiste-geometry
#    PRIVATE
#        WERKZEUGKISTE_EXPORT=
#)
#
#


# Fetch the library:
include(FetchContent)
FetchContent_Declare(
    werkzeugkiste
    GIT_REPOSITORY https://github.com/snototter/werkzeugkiste.git
    GIT_TAG main)
FetchContent_MakeAvailable(werkzeugkiste)
message(STATUS "werkzeugkiste v${werkzeugkiste_VERSION}")

set_target_properties(
    werkzeugkiste-geometry
    PROPERTIES
        POSITION_INDEPENDENT_CODE ON
)

target_link_libraries(
    ${pyzeugkiste_PYMODULE_NAME}
    PUBLIC
        werkzeugkiste::geometry
)


target_compile_definitions(${pyzeugkiste_PYMODULE_NAME}
    PRIVATE
        pyzeugkiste_VERSION_INFO=${pyzeugkiste_VERSION}
        pyzeugkiste_PYMODULE_NAME=${pyzeugkiste_PYMODULE_NAME}
)


set_target_properties(
    ${pyzeugkiste_PYMODULE_NAME}
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

message(STATUS "werkzeugkiste v${werkzeugkiste_VERSION}")
message(STATUS "pyzeugkiste v${pyzeugkiste_VERSION}")

