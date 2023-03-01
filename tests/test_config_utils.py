import pytest
import math
import datetime
from pyzeugkiste import config as pyc

# TODO test keys/parameter_names
# TODO test adjust paths
# TODO test placeholders
# TODO test loading nested TOML

def test_placeholders():
    cfg = pyc.load_toml_str("""
        str1 = 'value'
        str2 = '[%REP%]'

        [tbl]
        str1 = 'value %REP%'
        str2 = '123'
        str3 = 'key'
        """)

    cfg.replace_placeholders([('%REP%', '...'), ('e', '')])
    assert cfg.get_str('str1') == 'valu'
    assert cfg.get_str('str2') == '[...]'
    assert cfg.get_str('tbl.str1') == 'valu ...'
    assert cfg.get_str('tbl.str2') == '123'
    assert cfg.get_str('tbl.str3') == 'ky'
