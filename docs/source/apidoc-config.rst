===================
Configuration Utils
===================

This module provides a unified handling of different configuration formats
via the :class:`~pyzeugkiste.config.Config` class.
It supports `TOML <https://toml.io/en/>`__, `JSON <https://www.json.org/>`__
and `libconfig <http://hyperrealm.github.io/libconfig/>`__ formats.


----------
Quickstart
----------

.. code-block:: python

   from pyzeugkiste import config as pyc
   import datetime

   cfg = pyc.load_toml_str("""
       import-demo = 'path/to/config.json'

       [scalars]
       flag = true
       str = 'value'
       int = 1234
       flt1 = 1.0
       flt2 = 1e-3

       [compound]
       lst1 = [-42, 3, 1.5]
       lst2 = [[1, 2], 3, 4, 'five']

       [datetime]
       day = 2023-02-12
       time = 08:30:00

       [path-demo]
       path1 = 'a/relative/path'
       path2 = 'another/path'
       path3 = '/tmp'

       [replacement-demo]
       api_token = '$TOKEN'
       folder = '%%DEST%%/storage.out'
       """)

   # Does a parameter/group exist?
   assert 'scalars' in cfg
   assert 'scalars.str' in cfg
   assert 'str' in cfg['scalars']

   # Query a parameter
   # Option 1:
   cfg['scalars.flag']     # -> bool
   cfg['scalars']['flag']  # -> bool
   cfg['scalars.str']      # -> str
   cfg['scalars']['str']   # -> str

   # Option 2, explicitly typed:
   cfg.bool('scalars.flag')     # -> bool
   cfg['scalars'].bool('flag')  # -> bool
   cfg.str('scalars.str')       # -> str
   cfg['scalars'].str('str')    # -> str

   # Non-existing keys:
   cfg['unknown']             # Raises a KeyError
   cfg.int_or('unknown', 42)  # Returns int(42)

   # Numeric casts are performed if the value can be exactly
   # represented in the target type:
   cfg['scalars.int']        # -> int
   cfg.float('scalars.int')  # -> float(1234.0)
   cfg.int('scalars.flt1')   # -> int(1)
   cfg.int('scalars.flt2')   # Raises a TypeError (flt2 is 0.001)

   # Updating a parameter:
   cfg['scalars.str'] = 'new value'
   cfg['scalars']['str'] = '...'

   cfg['scalars.flt1'] = ''  # Raises a TypeError (can't change the type)

   # Create a parameter
   cfg['new-int'] = 42
   cfg['today'] = datetime.date.today()

   # Replace string placeholders
   cfg.replace_placeholders([('$TOKEN', 'abc'), ('%%DEST%%', 'path/to/dir')])
   # cfg['replacement-demo'] is now:
   #   api_token = 'abc'
   #   folder = 'path/to/dir/storage.out'

   # Adjust paths
   cfg.adjust_relative_paths('my-base-folder', ['path*'])
   # cfg['path-demo'] is now:
   #   path1 = 'my-base-folder/a/relative/path'
   #   path2 = 'my-base-folder/another/path'
   #   path3 = '/tmp'

   # Load/Import a configuration
   cfg.load_nested('import-demo')
   # cfg['import-demo'] is now a group, holding all parameters that were
   # specified in the referenced configuration file.

   # Represent a configuration in different formats:
   toml_str = cfg.to_toml()
   json_str = cfg.to_json()
   yaml_str = cfg.to_yaml()
   cfg_str = cfg.to_libconfig()
   # Or convert it to a plain dictionary:
   pydict = cfg.to_dict()


-------------
Documentation
-------------

.. autosummary::
   :nosignatures:

   ~pyzeugkiste.config.Config
   ~pyzeugkiste.config.ConfigType
   ~pyzeugkiste.config.NullValuePolicy
   ~pyzeugkiste.config.load
   ~pyzeugkiste.config.load_toml_file
   ~pyzeugkiste.config.load_toml_str
   ~pyzeugkiste.config.load_json_file
   ~pyzeugkiste.config.load_json_str
   ~pyzeugkiste.config.load_libconfig_file
   ~pyzeugkiste.config.load_libconfig_str
   ~pyzeugkiste.config.KeyError
   ~pyzeugkiste.config.TypeError
   ~pyzeugkiste.config.ValueError
   ~pyzeugkiste.config.ParseError

...................
Configuration Class
...................

.. autoclass:: pyzeugkiste.config.Config
   :autosummary:
   :autosummary-nosignatures:
   :members:

...................
Configuration Types
...................

.. autoclass:: pyzeugkiste.config.ConfigType

.........................
Handling None/Null Values
........................-

.. autoclass:: pyzeugkiste.config.NullValuePolicy

.......................
Loading a Configuration
.......................

.. autofunction:: pyzeugkiste.config.load

.. autofunction:: pyzeugkiste.config.load_toml_file

.. autofunction:: pyzeugkiste.config.load_toml_str

.. autofunction:: pyzeugkiste.config.load_json_file

.. autofunction:: pyzeugkiste.config.load_json_str

.. autofunction:: pyzeugkiste.config.load_libconfig_file

.. autofunction:: pyzeugkiste.config.load_libconfig_str

..........
Exceptions
..........

.. autoclass:: pyzeugkiste.config.KeyError
   :autosummary:

.. autoclass:: pyzeugkiste.config.TypeError
   :autosummary:

.. autoclass:: pyzeugkiste.config.ValueError
   :autosummary:

.. autoclass:: pyzeugkiste.config.ParseError
   :autosummary:
