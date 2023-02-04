.. _installation:

============
Installation
============

----------
OS Support
----------

The main target operating systems are GNU/Linux distributions.
The python package and test suite are also tested on macOS via
continuous integration, check the status at 
`the code repository <https://github.com/snototter/pyzeugkiste>`__.

Windows builds currently fail. Once these issues have been fixed in
`werkzeugkiste <https://github.com/snototter/werkzeugkiste>`__, Windows
support will be added to `pyzeugkiste` as well.
Note, however, that this is a rather low priority extension.


------------
Requirements
------------

To build this utility package, the following components are required:

#. A C++ compiler supporting at least ``C++17``.
#. `CMake \>= 3.15 <https://cmake.org/>`__ and a
   `compatible build tool <https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html>`__,
   like `Make <https://www.gnu.org/software/make/>`__, `Ninja <https://ninja-build.org/>`__,
   *etc.* For example, on GNU/Linux distributions, you'd simply need:

   .. code-block:: console

     # Debian & derivatives (like Ubuntu)
     sudo apt install cmake build-essentials ninja-build
    
     # On Fedora & openSUSE distributions, you need to install the same
     # packages via your package manager, i.e. `yum` or `zypper`.
#. The header-only `Eigen linear algebra library <https://eigen.tuxfamily.org/>`__.
  
   .. code-block:: console

      # Debian & derivatives
      sudo apt install libeigen3-dev

      # On Fedora & openSUSE distributions, the package name
      # is `eigen3-devel`.
#. `Python \>= 3.7 <https://www.python.org/>`_, along with its development
   library, *i.e.* system package ``python3-dev`` on Linux/macOS.
   On Windows, please follow the setup instructions of
   `pybind11 <https://pybind11.readthedocs.io/en/stable/basics.html>`__.
#. Python's standard package installer
   `pip \>= 10.0.0 <https://pypi.org/project/pip/>`_.


------------
Python Setup
------------

If all requirements are installed, the simplest way to install ``pyzeugkiste``
is to use the default package manager, ``pip``, which supports installation
directly from the github repository:

   .. code-block:: console
      :caption: Set up a virtual environment with up-to-date pip.

      python3 -m venv venv
      source venv/bin/activate
      python -m pip install -U pip
 
   .. code-block:: console
      :caption: Install ``pyzeugkiste``.

      python -m pip install git+https://github.com/snototter/pyzeugkiste.git
