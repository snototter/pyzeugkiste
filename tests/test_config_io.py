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


def test_load():
    # Check that 'load' correctly deduces the format from the file extension
    toml_file = data() / 'test-valid2.toml'
    cfg_toml = pyc.load_toml_file(toml_file)

    cfg = pyc.load(toml_file)
    assert cfg == cfg_toml

    json_file = data() / 'test-valid.json'
    cfg_json = pyc.load_json_file(json_file)

    cfg = pyc.load(json_file)
    assert cfg == cfg_json

    try:
        libcfg_file = data() / 'test-valid.cfg'
        cfg_libcfg = pyc.load_libconfig_file(libcfg_file)
        cfg = pyc.load(libcfg_file)
        assert cfg == cfg_libcfg
        # Also test libconfig de-/serialization:
        reloaded = pyc.load_libconfig_str(cfg.to_libconfig())
        assert reloaded == cfg
    except pyc.ParseError:
        assert False
    except RuntimeError:
        # Raised if libconfig is not available
        pass


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

    tstr = cfg.to_toml()
    t = toml.loads(tstr)
    # t.keys() returns only the names of the first level child nodes.
    pn1 = [p for p in cfg.list_parameter_names() if not '.' in p]
    assert len(t.keys()) == len(pn1)

    # Serialize as dict
    d = cfg.to_dict()
    # Workaround to "load" a dict:
    cfg2 = pyc.Config()
    cfg2['dict'] = d
    assert cfg2['dict'] == cfg

    # Serialize and reload as JSON
    jstr = cfg.to_json()
    # Is it parsable by the default JSON parser?
    j = json.loads(jstr)
    assert len(j) == len(pn1)
    # Load a config from the JSON string representation:
    cfg2 = pyc.load_json_str(cfg.to_json())
    assert isinstance(cfg2, type(cfg))
    # They differ, because the time parameter has been converted to string
    assert cfg != cfg2
    # Verify the conversion & delete this offending parameter
    assert isinstance(cfg['section1.time'], datetime.time)
    del cfg['section1.time']
    assert isinstance(cfg2['section1.time'], str)
    del cfg2['section1.time']
    # Now the configs should be equal
    assert cfg == cfg2

    # Serialize and reload as libconfig
    lc_str = cfg.to_libconfig()
    try:
        cfg2 = pyc.load_libconfig_str(lc_str)
        assert isinstance(cfg2, type(cfg))
        assert cfg == cfg2
    except pyc.ParseError:
        assert False
    except RuntimeError:
        # Raised if libconfig is not available
        pass


# TODO Add test for JSON + None/null


def test_nested():
    cfg = pyc.load_toml_file(data() / 'test-valid1.toml')

    # Nested TOML
    nested_toml = str(data() / 'test-valid2.toml')
    cfg_toml = pyc.load_toml_file(nested_toml)
    cfg['imports.toml'] = nested_toml

    cfg.load_nested('imports.toml')
    assert isinstance(cfg['imports.toml'], type(cfg))
    assert cfg['imports.toml'] == cfg_toml

    # Nested JSON
    nested_json = str(data() / 'test-valid.json')
    cfg_json = pyc.load_json_file(nested_json)
    cfg['imports']['json'] = nested_json

    # Alternative to cfg.load_nested('imports.json'):
    cfg['imports'].load_nested('json')
    assert isinstance(cfg['imports.json'], type(cfg))
    assert cfg['imports.json'] == cfg_json

    # Nested libconfig
    try:
        nested_libcfg = str(data() / 'test-valid.cfg')
        cfg_libcfg = pyc.load_libconfig_file(nested_libcfg)
        cfg['imports']['libconfig'] = nested_libcfg

        cfg['imports'].load_nested('libconfig')
        assert isinstance(cfg['imports']['libconfig'], type(cfg))
        assert cfg['imports']['libconfig'] == cfg_libcfg
    except pyc.ParseError:
        assert False
    except RuntimeError:
        # Raised if libconfig is not available
        pass
