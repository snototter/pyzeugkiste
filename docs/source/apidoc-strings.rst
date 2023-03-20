================
String Utilities
================

This module provides addition helpers to work with strings.

----------
Quickstart
----------

.. code-block:: python
   :caption: Basic usage

   from pyzeugkiste import strings as pst

    edit_dist = pst.levenshtein_distance('frobmorten', 'frambozzle')

    short_str = pst.shorten('0123456789', 6, ellipsis='...', pos=-1)

    slug = pst.slugify('String tO bE SluG1f13D!')


-------------
Documentation
-------------

.. autosummary::
   :nosignatures:

   pyzeugkiste.strings.levenshtein_distance
   pyzeugkiste.strings.shorten
   pyzeugkiste.strings.slugify


.. autofunction:: pyzeugkiste.strings.levenshtein_distance

.. autofunction:: pyzeugkiste.strings.shorten


