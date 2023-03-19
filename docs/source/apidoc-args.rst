================
Argument Parsing
================

This module provides custom validators to parse command line arguments.

----------
Quickstart
----------

.. code-block:: python
   :caption: Basic usage

   import argparse
   from pyzeugkiste import args as pya

   parser = argparse.ArgumentParser()
   parser.add_argument('day', action=pya.ValidateDate)
   parser.add_argument('--time', action=pya.ValidateTime, default=None)

   parser.parse_args(['2023-10-11', '--time', '10:30'])


-------------
Documentation
-------------

.. autosummary::
   :nosignatures:

   pyzeugkiste.args.ValidateDate
   pyzeugkiste.args.ValidateTime

...............
Date Validation
...............

.. autoclass:: pyzeugkiste.args.ValidateDate
   :autosummary:

...............
Time Validation
...............

.. autoclass:: pyzeugkiste.args.ValidateTime
   :autosummary:

