# Configuration file for the Sphinx documentation builder.
#
# For the full list of built-in configuration values, see the documentation:
# https://www.sphinx-doc.org/en/master/usage/configuration.html

# -- Project information -----------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#project-information

project = 'pyzeugkiste'
copyright = '2023, snototter'
author = 'snototter'

import pyzeugkiste
version = pyzeugkiste.__version__
release = version

# -- General configuration ---------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#general-configuration

extensions = [
  'sphinx.ext.autodoc',
  'sphinx.ext.autosummary',
  'sphinx.ext.napoleon',  # Parse Google and NumPy docstrings
  'sphinx.ext.mathjax',
  'sphinx.ext.intersphinx',
  'sphinx_copybutton',  # Copy button for code examples
  'autodocsumm', # adds TOC for autosummary: https://autodocsumm.readthedocs.io/en/latest/index.html
]

# -- Options for the extension modules ---------------------------------------

autosummary_generate = True

intersphinx_mapping = {
  'python': ('https://docs.python.org/', None),
  'numpy': ('http://docs.scipy.org/doc/numpy/', None)
}

# Add any paths that contain templates here, relative to this directory.
templates_path = ['_templates']

# List of patterns, relative to source directory, that match files and
# directories to ignore when looking for source files.
# This pattern also affects html_static_path and html_extra_path.
exclude_patterns = []



# -- Options for HTML output -------------------------------------------------
# https://www.sphinx-doc.org/en/master/usage/configuration.html#options-for-html-output

# html_theme = 'alabaster'
html_theme = "sphinx_rtd_theme"

html_static_path = ['_static']

# Theme options are theme-specific and customize the look and feel of a theme
# further.  For a list of options available for each theme, see the
# documentation.
# Documentation of the sphinx_rtd_theme: https://sphinx-rtd-theme.readthedocs.io/en/stable/configuring.html
html_theme_options = {
    "collapse_navigation" : False,
    "prev_next_buttons_location" : "both"
}

# Add any paths that contain custom static files (such as style sheets) here,
# relative to this directory. They are copied after the builtin static files,
# so a file named "default.css" will overwrite the builtin "default.css".
#html_static_path = []
html_static_path = ['_static']

# Include operators in autodoc
# Taken from https://stackoverflow.com/a/5599712/400948
def autodoc_skip_member(app, what, name, obj, would_skip, options):
    if name == "__init__":
        return False
    # Include overloaded operators:
    if name in ["__add__", "__call__", "__eq__", "__getstate__", "__iadd__", "__imul__",
            "__getitem__", "__setitem__", "__isub__", "__itruediv__", "__mul__", "__ne__", "__neg__", "__rmul__",
            "__sub__", "__truediv__", "__setstate__"]:
        return False
    return would_skip


def setup(app):
    # I want to include overloaded operators in autodoc
    app.connect("autodoc-skip-member", autodoc_skip_member)
    app.add_css_file("dark.css")
