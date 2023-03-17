from pyzeugkiste._core._cfg import (
    __doc__, Configuration,
    load_toml_str, load_toml_file,
    load_json_str, load_json_file,
    load_libconfig_str, load_libconfig_file,
    KeyError, TypeError, ValueError, ParseError
)

__module__ = "pyzeugkiste.config"
Configuration.__module__ = __module__
KeyError.__module__ = __module__
TypeError.__module__ = __module__
ValueError.__module__ = __module__
ParseError.__module__ = __module__
