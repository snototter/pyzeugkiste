===================
Configuration Utils
===================

----------
Quickstart
----------

.. warning::
   TODO Add summary + example

.....................
Configuration Example
.....................

.. code-block:: toml
   :caption: Exemplary TOML configuration

   [scalars]
   flag = true
   str = 'value'
   int = 1234
   flt1 = 1.0
   flt2 = -1e3

   [compound]
   # Mixed floating point and integral values:
   lst_numeric = [-42, 3, 1.5]

   [datetime]
   day = 2023-02-12
   time = 08:30:00

   [paths]
   # TODO path adjustments

   [replacement-demo]
   # TODO string replacements

   [fancy]
   # TODO inhomogeneous lists (once std::variant support is added)
   # TODO load nested configuration


...........
Basic Usage
...........

.. code-block:: python
   :caption: Basic usage

   from pyzeugkiste import config
   cfg = config.Configuration.load_toml_file('path/to/example.toml')

   # List all parameter names
   cfg.list_parameter_names()

   # Is a parameter set?
   'scalars.str' in cfg

   # Query a parameter
   cfg['scalars.flag']  # -> bool
   cfg['scalars.str']   # -> str
   cfg['scalars.flt1']  # -> float

   cfg.get_str('scalars.str')     # -> str
   cfg.get_float('scalars.flt1')  # -> float

   cfg['unknown']                 # Raises a KeyError
   cfg.get_int_or('unknown', 42)  # Returns int(42)

   # Numeric casts are performed if the value can be exactly
   # represented in the target type:
   cfg['scalars.int']            # -> int
   cfg.get_float('scalars.int')  # -> float(1234.0)
   cfg.get_int('scalars.flt1')   # -> int(1)
   cfg.get_int('scalars.flt2')   # Raises a TypeError

   # Update a parameter
   cfg['scalars.str'] = 'my new value'

   cfg['scalars.flt'] = ''  # Raises a TypeError (can't change the type)

   # Create a parameter
   cfg['new-int'] = 42
   cfg['today'] = datetime.date.today()


-------------
Documentation
-------------

.. autosummary::
   :nosignatures:

   pyzeugkiste.config.Configuration
   pyzeugkiste.config.KeyError
   pyzeugkiste.config.TypeError
   pyzeugkiste.config.ParseError

.............
Configuration
.............

.. autoclass:: pyzeugkiste.config.Configuration
   :autosummary:
   :autosummary-nosignatures:
   :members:

........
KeyError
........

.. autoclass:: pyzeugkiste.config.KeyError
   :autosummary:

.........
TypeError
.........

.. autoclass:: pyzeugkiste.config.TypeError
   :autosummary:

..........
ParseError
..........

.. autoclass:: pyzeugkiste.config.ParseError
   :autosummary:
