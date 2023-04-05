import pytest
import json
import math
import pytz
import toml
import datetime
from pathlib import Path
from pyzeugkiste import config as pyc


def data():
    return Path(__file__).parent.resolve() / 'data'


def test_io_toml():
    with pytest.raises(pyc.ParseError):
        pyc.load_toml_file(data() / 'test-invalid.toml')

    with pytest.raises(pyc.ParseError):
        pyc.load_toml_file('no-such-file.toml')

    cfg = pyc.Config()
    assert cfg.empty()

    cfg = pyc.load_toml_file(data() / 'test-valid1.toml')
    assert not cfg.empty()
    assert 'value1' in cfg
    assert 'value2' in cfg

    # Serialize and reload as TOML
    tstr = cfg.to_toml()
    cfg2 = pyc.load_toml_str(tstr)
    assert cfg == cfg2
    cfg['new-val'] = True
    assert cfg != cfg2

    pn1 = cfg.list_parameter_names()
    pn2 = cfg2.list_parameter_names()
    assert len(pn1) == (len(pn2) + 1)
    # TODO test .size

    tstr = cfg.to_toml()
    t = toml.loads(tstr)
    # t.keys() returns only the names of the first level child nodes.
    pn1 = [p for p in cfg.list_parameter_names() if not '.' in p]
    assert len(t.keys()) == len(pn1)

    # Serialize and reload as JSON
    jstr = cfg.to_json()
    j = json.loads(jstr)
    assert len(j) == len(pn1)

    # TODO reload Configuration as JSON

    # TODO serialize and reload as libconfig


# TODO
# def test_io_json()
# special test for None

# TODO
# def test_io_libconfig()
