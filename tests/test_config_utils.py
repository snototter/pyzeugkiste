import pytest
import math
import datetime
from pyzeugkiste import config as pyc

# TODO test keys/parameter_names
# TODO test adjust paths
# TODO test placeholders
# TODO test load_nested

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
    assert cfg.str('str1') == 'valu'
    assert cfg.str('str2') == '[...]'
    assert cfg.str('tbl.str1') == 'valu ...'
    tmp = cfg['tbl']
    assert tmp['str1'] == 'valu ...'
    assert cfg.str('tbl.str2') == '123'
    assert tmp['str2'] == '123'
    assert cfg.str('tbl.str3') == 'ky'
    assert tmp['str3'] == 'ky'
