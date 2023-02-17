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

    cfg = pyc.Configuration()
    assert cfg.empty()

    cfg = pyc.load_toml_file(data() / 'test-valid1.toml')
    assert not cfg.empty()
    assert 'value1' in cfg
    assert 'value2' in cfg

    # Serialize and reload as TOML
    tstr = cfg.to_toml()
    cfg2 = pyc.load_toml_str(tstr)
    assert cfg == cfg2
    cfg.set_bool('new', True)
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

def test_boolean():
    cfg = pyc.load_toml_str("""
        b1 = true
        b2 = false
        num = 3
        flt = 1.5
        str = 'value'
        """)

    with pytest.raises(pyc.KeyError):
        cfg.get_bool('b')

    with pytest.raises(pyc.TypeError):
        cfg.get_bool('num')

    with pytest.raises(pyc.TypeError):
        cfg.get_bool('flt')

    with pytest.raises(pyc.TypeError):
        cfg.get_bool('str')

    assert cfg.get_bool('b1')
    assert not cfg.get_bool('b2')
    assert cfg['b1']
    assert not cfg['b2']
    assert isinstance(cfg['b1'], bool)

    # Use default values if the key is not found:
    assert cfg.get_bool_or('b', True)
    assert not cfg.get_bool_or('b', False)

    with pytest.raises(pyc.TypeError):
        cfg.get_bool_or('str', True)

    # Changing a value is allowed
    cfg['b1'] = False
    assert not cfg['b1']

    # But changing the type is not allowed
    with pytest.raises(pyc.TypeError):
        cfg['str'] = True

    with pytest.raises(pyc.TypeError):
        cfg.set_bool('flt', True)


def test_integer():
    cfg = pyc.load_toml_str("""
        b = true
        i1 = 3
        i2 = -12345
        i3 = 21474836480
        i4 = -21474836480
        flt1 = 1.5
        flt2 = -3.0
        str = 'value'
        """)

    with pytest.raises(pyc.KeyError):
        cfg.get_int('i')

    with pytest.raises(pyc.TypeError):
        cfg.get_int('str')

    assert 3 == cfg.get_int('i1')
    assert 3 == cfg['i1']
    assert -12345 == cfg.get_int('i2')
    assert -12345 == cfg['i2']
    assert 21474836480 == cfg.get_int('i3')
    assert 21474836480 == cfg['i3']
    assert -21474836480 == cfg.get_int('i4')
    assert -21474836480 == cfg['i4']

    assert isinstance(cfg.get_int('i1'), int)
    assert isinstance(cfg['i1'], int)

    # Use default values if the key is not found:
    assert 42 == cfg.get_int_or('x', 42)
    assert -17 == cfg.get_int_or('x', -17)

    with pytest.raises(pyc.TypeError):
        cfg.get_int_or('str', 3)

    # Implicit casts are allowed if the value is exactly
    # representable in the target type:
    with pytest.raises(pyc.TypeError):
        cfg.get_int('flt1')

    assert -3 == cfg.get_int('flt2')
    assert isinstance(cfg.get_int('flt2'), int)
    assert isinstance(cfg['flt2'], float)

    # Set an integer
    cfg.set_int('my-int1', -17)
    assert 'my-int1' in cfg
    assert isinstance(cfg['my-int1'], int)
    assert -17 == cfg['my-int1']

    cfg['my-int2'] = 987654321
    assert 'my-int2' in cfg
    assert isinstance(cfg['my-int2'], int)
    assert 987654321 == cfg['my-int2']

    # Changing a value is allowed
    cfg['my-int1'] = 3
    assert 3 == cfg['my-int1']

    # But changing the type is not allowed
    with pytest.raises(pyc.TypeError):
        cfg['str'] = 4

    with pytest.raises(pyc.TypeError):
        cfg.set_int('b', 2)

    # Similar to querying, however, if a value can be exactly represented
    # by the target type, it is implicitly cast:
    cfg['flt1'] = 3
    assert pytest.approx(3) == cfg['flt1']
    assert isinstance(cfg['flt1'], float)  # But the type is not changed

    # We also cannot change a numeric type:
    with pytest.raises(pyc.TypeError):
        cfg.set_int('flt2', -1)

    with pytest.raises(pyc.TypeError):
        cfg.set_float('my-int1', 42)

    # But it can be set if the value is exactly representable:
    cfg['flt2'] = int(-1)
    assert pytest.approx(-1) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)

    cfg['my-int1'] = float(42)
    assert 42 == cfg['my-int1']
    assert isinstance(cfg['my-int1'], int)


def test_floating_point():
    cfg = pyc.load_toml_str("""
        b = true
        i = 42
        flt1 = 1.5
        flt2 = -3.0
        flt3 = -1e19
        spec1 = -inf
        spec2 = nan
        str = 'value'
        """)

    with pytest.raises(pyc.KeyError):
        cfg.get_float('x')

    with pytest.raises(pyc.TypeError):
        cfg.get_int('str')

    assert pytest.approx(1.5) == cfg.get_float('flt1')
    assert pytest.approx(1.5) == cfg['flt1']
    assert pytest.approx(-3.0) == cfg.get_float('flt2')
    assert pytest.approx(-3.0) == cfg['flt2']
    assert pytest.approx(-1e19) == cfg.get_float('flt3')
    assert pytest.approx(-1e19) == cfg['flt3']

    assert isinstance(cfg.get_float('flt1'), float)
    assert isinstance(cfg['flt1'], float)

    assert math.isinf(cfg['spec1'])
    assert cfg['spec1'] < 0.0
    assert math.isnan(cfg['spec2'])

    # Use default values if the key is not found:
    assert pytest.approx(42.8) == cfg.get_float_or('x', 42.8)
    assert pytest.approx(-17.5) == cfg.get_float_or('x', -17.5)

    with pytest.raises(pyc.TypeError):
        cfg.get_float_or('str', 3.5)

    # Implicit casts are allowed if the value is exactly
    # representable in the target type:
    assert pytest.approx(42.0) == cfg.get_float('i')
    assert isinstance(cfg.get_float('i'), float)
    assert isinstance(cfg['i'], int)

    # Set a float
    cfg.set_float('my-flt1', 1.5e4)
    assert 'my-flt1' in cfg
    assert isinstance(cfg['my-flt1'], float)
    assert pytest.approx(1.5e4) == cfg['my-flt1']

    cfg['my-flt2'] = 1234.5678
    assert 'my-flt2' in cfg
    assert isinstance(cfg['my-flt2'], float)
    assert pytest.approx(1234.5678) == cfg['my-flt2']

    # Changing a value is allowed
    cfg['my-flt1'] = 17.5
    assert pytest.approx(17.5) == cfg['my-flt1']

    # But changing the type is not allowed
    with pytest.raises(pyc.TypeError):
        cfg['str'] = 20.5

    with pytest.raises(pyc.TypeError):
        cfg.set_float('b', 2.0)

    # Similar to querying, however, if a value can be exactly represented
    # by the target type, it is implicitly cast:
    cfg['i'] = 3.0
    assert pytest.approx(3.0) == cfg.get_float('i')
    assert 3 == cfg['i']
    assert isinstance(cfg['i'], int)  # But the type is not changed

    cfg.set_float('flt2', -1)
    assert pytest.approx(-1) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)

    cfg['flt2'] = 1234
    assert pytest.approx(1234.0) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)

    cfg['flt1'] = math.inf
    assert math.isinf(cfg['flt1'])
    assert cfg['flt1'] > 0

# TODO test_str
# TODO test_date
# TODO test_time

def test_datetime():
    cfg = pyc.load_toml_str("""
        day = 2022-12-01
        time = 08:30:00
        dt1 = 2000-02-29T17:30:15.123
        dt2 = 2000-02-29T17:30:15.123-12:00
        """)
    # Querying a date/time/datetime
    day = datetime.date(2022, 12, 1)
    assert cfg['day'] == day
    assert cfg.get_date('day') == day

    tm = datetime.time(8, 30)
    assert cfg['time'] == tm
    assert cfg.get_time('time') == tm

    dt = datetime.datetime(2000, 2, 29, 17, 30, 15, 123000)
    assert isinstance(cfg['dt1'], datetime.datetime)
    assert cfg['dt1'] == dt
    assert cfg.get_datetime('dt1') == dt
    assert cfg['dt1'].tzinfo is None

    with pytest.raises(pyc.KeyError):
        cfg.get_datetime('no-such-key')
    assert cfg.get_datetime_or('no-such-key', dt) == dt

    # A date/time cannot be implicitly converted to a datetime
    with pytest.raises(pyc.TypeError):
        cfg.get_datetime('day')
    with pytest.raises(pyc.TypeError):
        cfg.get_datetime_or('time', dt)

    # pyzeugkiste doesn't know the timezone, only the offset. Thus, the
    # datetime object will be adjusted to UTC time, if an offset has been
    # specified
    assert cfg['dt1'].tzinfo is None
    assert cfg['dt2'].tzinfo is not None
    assert cfg['dt2'].utcoffset().total_seconds() == 0

    # TODO test time offsets

    # Setting a datetime
    with pytest.raises(pyc.TypeError):
        cfg['dt1'] = day
    with pytest.raises(pyc.TypeError):
        cfg['dt1'] = tm


# TODO test keys/parameter_names
# TODO test adjust paths
# TODO test placeholders
# TODO test loading nested TOML

# def test_placeholders():
#     cfg = config.Configuration.load_toml_string("""
#         str1 = 'value'
#         str2 = '[%REP%]'

#         [tbl]
#         str1 = 'value %REP%'
#         str2 = '123'
#         str3 = 'key'
#         """)

#     cfg.replace_placeholders([('%REP%', '...'), ('e', '')])
#     assert cfg.get_str('str1') == 'valu'
#     assert cfg.get_str('str2') == '[...]'
#     assert cfg.get_str('tbl.str1') == 'valu ...'
#     assert cfg.get_str('tbl.str2') == '123'
#     assert cfg.get_str('tbl.str3') == 'ky'
