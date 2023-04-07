import pytest
import math
import datetime
from pyzeugkiste import config as pyc

def test_boolean():
    cfg = pyc.load_toml_str("""
        b1 = true
        b2 = false
        num = 3
        flt = 1.5
        str = 'value'
        """)

    assert len(cfg) == 5

    # Invalid keys --> KeyError
    with pytest.raises(pyc.KeyError):
        cfg['b']
    with pytest.raises(KeyError):
        cfg['b']

    with pytest.raises(pyc.KeyError):
        cfg.bool('b')
    with pytest.raises(KeyError):
        cfg.bool('b')

    # Try to load as an invalid/incompatible type --> TypeError
    with pytest.raises(pyc.TypeError):
        cfg.bool('num')
    with pytest.raises(TypeError):
        cfg.bool('num')

    with pytest.raises(pyc.TypeError):
        cfg.bool('flt')
    with pytest.raises(TypeError):
        cfg.bool('flt')

    with pytest.raises(pyc.TypeError):
        cfg.bool('str')

    # Retrieve value as intended:
    assert cfg['b1']
    assert not cfg['b2']
    assert cfg.bool('b1')
    assert not cfg.bool('b2')
    assert isinstance(cfg['b1'], bool)
    assert isinstance(cfg.bool('b1'), bool)

    # Use default values if the key is not found:
    assert cfg.bool_or('b', True)
    assert not cfg.bool_or('b', False)

    # ... but an invalid type remains an invalid type:
    with pytest.raises(pyc.TypeError):
        cfg.bool_or('str', True)

    # Changing a value is allowed
    cfg['b1'] = False
    assert not cfg['b1']

    # But changing the type is not allowed
    with pytest.raises(pyc.TypeError):
        cfg['str'] = True

    with pytest.raises(pyc.TypeError):
        cfg['flt'] = True


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

    assert len(cfg) == 8

    # Invalid keys --> KeyError
    with pytest.raises(pyc.KeyError):
        cfg.int('i')

    with pytest.raises(pyc.TypeError):
        cfg.int('str')

    # Use as intended
    assert 3 == cfg['i1']
    assert 3 == cfg.int('i1')
    assert -12345 == cfg['i2']
    assert -12345 == cfg.int('i2')
    assert 21474836480 == cfg['i3']
    assert 21474836480 == cfg.int('i3')
    assert -21474836480 == cfg['i4']
    assert -21474836480 == cfg.int('i4')

    assert isinstance(cfg['i1'], int)
    assert isinstance(cfg.int('i1'), int)

    # Use default values if the key is not found:
    assert 42 == cfg.int_or('x', 42)
    assert -17 == cfg.int_or('x', -17)

    # ... but an invalid type remains an invalid type:
    with pytest.raises(pyc.TypeError):
        cfg.int_or('str', 3)

    # Implicit casts are allowed if the value is exactly
    # representable by the target type:
    with pytest.raises(pyc.TypeError):
        cfg.int('flt1')  # 1.5 can't be represented as int

    assert -3 == cfg.int('flt2')
    assert isinstance(cfg.int('flt2'), int)
    assert isinstance(cfg['flt2'], float)

    # Set an integer
    cfg['my-int1'] = -17
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
        cfg['b'] = 2

    # Similar to querying, however, if a value can be exactly represented
    # by the target type, it is implicitly cast:
    cfg['flt1'] = 3
    assert pytest.approx(3) == cfg['flt1']
    assert isinstance(cfg['flt1'], float)  # But the type is not changed

    cfg['flt2'] = -1
    assert pytest.approx(-1) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)
    assert -1 == cfg.int('flt2')

    cfg['flt2'] = int(-3)
    assert pytest.approx(-3) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)

    with pytest.raises(pyc.TypeError):
        cfg['my-int1'] = 4.2

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

    assert len(cfg) == 8

    with pytest.raises(pyc.KeyError):
        cfg.float('x')

    with pytest.raises(pyc.TypeError):
        cfg.int('str')

    assert pytest.approx(1.5) == cfg.float('flt1')
    assert pytest.approx(1.5) == cfg['flt1']
    assert pytest.approx(-3.0) == cfg.float('flt2')
    assert pytest.approx(-3.0) == cfg['flt2']
    assert pytest.approx(-1e19) == cfg.float('flt3')
    assert pytest.approx(-1e19) == cfg['flt3']

    assert isinstance(cfg.float('flt1'), float)
    assert isinstance(cfg['flt1'], float)

    assert math.isinf(cfg['spec1'])
    assert cfg['spec1'] < 0.0
    assert math.isnan(cfg['spec2'])

    # Use default values if the key is not found:
    assert pytest.approx(42.8) == cfg.float_or('x', 42.8)
    assert pytest.approx(-17.5) == cfg.float_or('x', -17.5)

    with pytest.raises(pyc.TypeError):
        cfg.float_or('str', 3.5)

    # Implicit casts are allowed if the value is exactly
    # representable in the target type:
    assert pytest.approx(42.0) == cfg.float('i')
    assert isinstance(cfg.float('i'), float)
    assert isinstance(cfg['i'], int)

    # Set a float
    cfg['my-flt1'] = 1.5e4
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
        cfg['b'] = 2.0

    # Similar to querying, however, if a value can be exactly represented
    # by the target type, it is implicitly cast:
    cfg['i'] = 3.0
    assert pytest.approx(3.0) == cfg.float('i')
    assert 3 == cfg['i']
    assert isinstance(cfg['i'], int)  # But the type is not changed

    cfg['flt2'] = -1
    assert pytest.approx(-1) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)

    cfg['flt2'] = 1234
    assert pytest.approx(1234.0) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)

    cfg['flt1'] = math.inf
    assert math.isinf(cfg['flt1'])
    assert cfg['flt1'] > 0

def test_str():
    cfg = pyc.load_toml_str("""
        flag = true
        num = 1.5
        str = 'value'
        """)
    assert len(cfg) == 3

    # Try to load as an invalid/incompatible type --> TypeError
    with pytest.raises(pyc.TypeError):
        cfg.str('flag')
    with pytest.raises(TypeError):
        cfg.str('flag')

    with pytest.raises(pyc.TypeError):
        cfg.str('num')
    with pytest.raises(TypeError):
        cfg.str('num')

    with pytest.raises(pyc.TypeError):
        cfg.bool('str')

    with pytest.raises(pyc.TypeError):
        cfg.int('str')

    # Retrieve value as intended:
    assert 'value' == cfg['str']
    assert 'value' == cfg.str('str')
    assert isinstance(cfg['str'], str)
    assert isinstance(cfg.str('str'), str)

    # Use default values if the key is not found:
    assert 'A' == cfg.str_or('unknown', 'A')

    # ... but an invalid type remains an invalid type:
    with pytest.raises(pyc.TypeError):
        cfg.str_or('flag', 'A')

    # Changing a value is allowed
    cfg['str'] = 'changed'
    assert 'changed' == cfg['str']

    # But changing the type is not allowed
    with pytest.raises(pyc.TypeError):
        cfg['str'] = True

    with pytest.raises(pyc.TypeError):
        cfg['num'] = '123'


def test_date():
    with pytest.raises(pyc.ParseError):
        pyc.load_toml_str("day = 2023-02-29")

    cfg = pyc.load_toml_str("""
        day = 2022-12-01
        time = 08:30:00
        dt = 2000-02-29T17:30:15.123
        """)
    assert len(cfg) == 3

    # Querying a date
    day = datetime.date(2022, 12, 1)
    assert cfg['day'] == day
    assert cfg.date('day') == day
    assert isinstance(cfg['day'], datetime.date)

    with pytest.raises(pyc.KeyError):
        cfg.date('no-such-key')

    with pytest.raises(pyc.TypeError):
        cfg.time('day')

    with pytest.raises(pyc.TypeError):
        cfg.datetime('day')

    with pytest.raises(pyc.TypeError):
        cfg['day'] = datetime.time(8, 30)

    day += datetime.timedelta(days=3)
    cfg['day'] = day
    assert cfg['day'] == day


def test_time():
    with pytest.raises(pyc.ParseError):
        pyc.load_toml_str("tm = 24:30:01")

    cfg = pyc.load_toml_str("""
        time = 08:30:01
        dt = 2000-02-29T17:30:15.123
        """)
    assert len(cfg) == 2

    # Querying a time
    tm = datetime.time(8, 30, 1)
    assert cfg['time'] == tm
    assert cfg.time('time') == tm
    assert isinstance(cfg['time'], datetime.time)

    with pytest.raises(pyc.KeyError):
        cfg.time('no-such-key')

    with pytest.raises(pyc.TypeError):
        cfg.date('time')

    with pytest.raises(pyc.TypeError):
        cfg.datetime('time')

    with pytest.raises(pyc.TypeError):
        cfg['time'] = datetime.date.today()

    tm = datetime.time(23, 59, 59, 135)
    assert cfg['time'] != tm
    cfg['time'] = tm
    assert cfg['time'] == tm


def test_datetime():
    cfg = pyc.load_toml_str("""
        day = 2022-12-01
        time = 08:30:00
        dt1 = 2000-02-29T17:30:15.123
        dt2 = 2000-02-29T17:30:15.123Z
        dt3 = 2000-02-29T17:30:15.123+00:00
        dt4 = 2000-02-29T17:30:15.123-12:10
        dt5 = 2000-02-29T17:30:15.123+00:01
        """)
    assert len(cfg) == 7

    dt = datetime.datetime(2000, 2, 29, 17, 30, 15, 123000)
    assert isinstance(cfg['dt1'], datetime.datetime)
    assert cfg['dt1'] == dt
    assert cfg.datetime('dt1') == dt
    assert cfg['dt1'].tzinfo is None

    with pytest.raises(pyc.KeyError):
        cfg.datetime('no-such-key')
    assert cfg.datetime_or('no-such-key', dt) == dt

    # A date/time cannot be implicitly converted to a datetime
    with pytest.raises(pyc.TypeError):
        cfg.datetime('day')
    with pytest.raises(pyc.TypeError):
        cfg.datetime_or('time', dt)

    # pyzeugkiste will set the offset of the datetime's timezone info
    assert cfg['dt1'].tzinfo is None
    assert cfg['dt2'].tzinfo is not None
    assert cfg['dt3'].tzinfo is not None
    assert cfg['dt4'].tzinfo is not None
    assert cfg['dt5'].tzinfo is not None

    # -12:10 hrs = 43,800
    assert pytest.approx(0.0) == cfg['dt2'].utcoffset().total_seconds()
    assert pytest.approx(0.0) == cfg['dt3'].utcoffset().total_seconds()
    assert pytest.approx(-43800.0) == cfg['dt4'].utcoffset().total_seconds()
    assert pytest.approx(60.0) == cfg['dt5'].utcoffset().total_seconds()

    assert cfg['dt2'] == cfg['dt3']
    assert cfg['dt2'] == (cfg['dt4'] - datetime.timedelta(hours=12, minutes=10))
    assert cfg['dt2'] == (cfg['dt5'] + datetime.timedelta(minutes=1))

    # Setting a datetime
    with pytest.raises(pyc.TypeError):
        cfg['dt1'] = datetime.date(2022, 12, 1)
    with pytest.raises(pyc.TypeError):
        cfg['dt1'] = datetime.time(8, 30)
