#TODO build setup.py based on the CMake example! https://github.com/pybind/cmake_example/blob/master/setup.py

import pathlib

# Available at setup time due to pyproject.toml
from pybind11 import get_cmake_dir
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup


def load_version():
    here = pathlib.Path(__file__).parent.resolve()
    return (here / "VERSION").read_text().strip()


def load_long_description():
    here = pathlib.Path(__file__).parent.resolve()
    return (here / "README.md").read_text(encoding="utf-8")

__version__ = load_version()


from glob import glob


# The main interface is through Pybind11Extension.
# * You can add cxx_std=11/14/17, and then build_ext can be removed.
# * You can set include_pybind11=false to add the include directory yourself,
#   say from a submodule.
#
# Note:
#   Sort input source files if you glob sources to ensure bit-for-bit
#   reproducible builds (https://github.com/pybind/python_example/pull/53)

here = pathlib.Path(__file__).parent.resolve()
#here / "include"

ext_modules = [
    Pybind11Extension("pyzeugkiste",
#        sorted(glob("src/*.cpp")),
        ["src/pyzeugkiste.cpp",
#        "include/werkzeugkiste-bindings/vector_bindings.h",
        ],
        define_macros = [('pyzeugkiste_VERSION_INFO', __version__)],
#        include_directories = [ get_cmake_dir() + "/include/werkzeugkiste-bindings"],
        include_dirs=["include/", str(here / "include")],
        cxx_std=17,
#        include_pybind11=False
        ),
]

print(f'TODO {ext_modules[0].include_dirs}')

setup(
    name="pyzeugkiste",
    version=__version__,
    author="snototter",
    author_email="snototter@users.noreply.github.com",
    description="TODO.",
    long_description=load_long_description(),
    url="TODO",
    ext_modules=ext_modules,
    extras_require={"test": "pytest"},
    # Currently, build_ext only provides an optional "highest supported C++
    # level" feature, but in the future it may provide more features.
   # cmdclass={"build_ext": build_ext},
    zip_safe=False,
    install_requires=[
        "numpy>=1.7.0"
    ],
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires=">=3.7",
)

