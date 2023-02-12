import pytest
from pyzeugkiste import config

# TODO Tests missing, e.g. list_parameter_names, lead_nested_toml, ...

def test_scalars():
    cfg = config.Configuration.load_toml_string("""
        int = 23
        flt = 1.5
        str = "value"
        bool = false
        """)

    ###########################################################################
    # Boolean
    assert not cfg.get_bool('bool')
    assert not cfg['bool']
    assert isinstance(cfg['bool'], bool)

    # -- Implicit type conversion is not allowed
    with pytest.raises(config.TypeError):
        cfg.get_bool('int')
    with pytest.raises(config.TypeError):
        cfg.get_bool('flt')
    with pytest.raises(config.TypeError):
        cfg.get_bool('str')

    # -- Raises an error if the key cannot be found
    with pytest.raises(config.KeyError):
        cfg.get_bool('nested.scalars.b')
    with pytest.raises(config.KeyError):
        cfg['nested.scalars.b']

    # -- Solution 1) Set the missing property
    cfg.set_bool('nested.scalars.bool', value = True)
    #TODO also set via __setitem__
    assert cfg.get_bool('nested.scalars.bool')

    # -- Solution 2) Explicitly request a default value
    assert cfg.get_bool_or("no-such-key", default_value = True)
    assert not cfg.get_bool_or("no-such-key", False)

    ###########################################################################
    # Strings
    assert cfg.get_str('str') == 'value'

    with pytest.raises(config.TypeError):
        cfg.get_str('nested.scalars.bool')

    with pytest.raises(config.KeyError):
        cfg.get_str('nested.scalars.str')

    cfg.set_str('nested.scalars.str', 'frobmorten')
    assert cfg.get_str("nested.scalars.str") == 'frobmorten'

    assert cfg.get_bool_or("no-such-key", True)
    assert not cfg.get_bool_or("no-such-key", False)

def test_representations():
    #TODO load an extensive toml example instead!
    cfg = config.Configuration.load_toml_string("""
        int = 23
        flt = 1.5
        str = "value"
        bool = false
        """)
    other = config.Configuration.load_toml_string(cfg.to_toml())
    assert cfg == other

    with pytest.raises(RuntimeError):
        config.Configuration.load_toml_string(cfg.to_json())


def test_placeholders():
    cfg = config.Configuration.load_toml_string("""
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
