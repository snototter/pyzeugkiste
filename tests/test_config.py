import pytest
import math
import datetime
from pyzeugkiste import config as pyc

def test_exception_hierarchy():
    assert issubclass(pyc.KeyError, KeyError)
    assert issubclass(pyc.TypeError, TypeError)
    assert issubclass(pyc.ParseError, RuntimeError)
    assert issubclass(pyc.ValueError, ValueError)


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
        cfg.get_bool('b')
    with pytest.raises(KeyError):
        cfg.get_bool('b')

    # Try to load as an invalid/incompatible type --> TypeError
    with pytest.raises(pyc.TypeError):
        cfg.get_bool('num')
    with pytest.raises(TypeError):
        cfg.get_bool('num')

    with pytest.raises(pyc.TypeError):
        cfg.get_bool('flt')
    with pytest.raises(TypeError):
        cfg.get_bool('flt')

    with pytest.raises(pyc.TypeError):
        cfg.get_bool('str')

    # Retrieve value as intended:
    assert cfg['b1']
    assert not cfg['b2']
    assert cfg.get_bool('b1')
    assert not cfg.get_bool('b2')
    assert isinstance(cfg['b1'], bool)
    assert isinstance(cfg.get_bool('b1'), bool)

    # Use default values if the key is not found:
    assert cfg.get_bool_or('b', True)
    assert not cfg.get_bool_or('b', False)

    # ... but an invalid type remains an invalid type:
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

    assert len(cfg) == 8

    # Invalid keys --> KeyError
    with pytest.raises(pyc.KeyError):
        cfg.get_int('i')

    with pytest.raises(pyc.TypeError):
        cfg.get_int('str')

    # Use as intended
    assert 3 == cfg['i1']
    assert 3 == cfg.get_int('i1')
    assert -12345 == cfg['i2']
    assert -12345 == cfg.get_int('i2')
    assert 21474836480 == cfg['i3']
    assert 21474836480 == cfg.get_int('i3')
    assert -21474836480 == cfg['i4']
    assert -21474836480 == cfg.get_int('i4')

    assert isinstance(cfg['i1'], int)
    assert isinstance(cfg.get_int('i1'), int)

    # Use default values if the key is not found:
    assert 42 == cfg.get_int_or('x', 42)
    assert -17 == cfg.get_int_or('x', -17)

    # ... but an invalid type remains an invalid type:
    with pytest.raises(pyc.TypeError):
        cfg.get_int_or('str', 3)

    # Implicit casts are allowed if the value is exactly
    # representable by the target type:
    with pytest.raises(pyc.TypeError):
        cfg.get_int('flt1')  # 1.5 can't be represented as int

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

    cfg.set_int('flt2', -1)
    assert pytest.approx(-1) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)
    assert -1 == cfg.get_int('flt2')

    cfg['flt2'] = int(-3)
    assert pytest.approx(-3) == cfg['flt2']
    assert isinstance(cfg['flt2'], float)

    with pytest.raises(pyc.TypeError):
        cfg.set_float('my-int1', 4.2)

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
# TODO test_time
# TODO test_group

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
    assert cfg.get_date('day') == day
    assert isinstance(cfg['day'], datetime.date)

    with pytest.raises(pyc.KeyError):
        cfg.get_date('no-such-key')

    with pytest.raises(pyc.TypeError):
        cfg.get_time('day')

    with pytest.raises(pyc.TypeError):
        cfg.get_datetime('day')

    with pytest.raises(pyc.TypeError):
        cfg['day'] = datetime.time(8, 30)

    day += datetime.timedelta(days=3)
    cfg['day'] = day
    assert cfg['day'] == day


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


def test_size():
    cfg = pyc.load_toml_str("""
        [scalars]
        flag = true
        str = 'value'
        int = 1234
        flt1 = 1.0
        flt2 = -1e3

        [compound]
        # Mixed floating point and integral values:
        lst_numeric = [-42, 3, 1.5]

        [datetime]
        day = 2023-02-12
        time = 08:30:00
        """)
    assert 3 == len(cfg)
    assert 5 == len(cfg['scalars'])
    assert 1 == len(cfg['compound'])
    # TODO enable once we support list getter/setter
    # assert 3 == len(cfg['compound.lst_numeric'])
    assert 2 == len(cfg['datetime'])

    assert 'scalars.str' in cfg
    assert 'str' in cfg['scalars']

    assert 'scalars.flt' not in cfg
    assert 'scalars.flt1' in cfg

    assert 'flt' not in cfg['scalars']
    assert 'flt1' in cfg['scalars']

    # Raises a standard TypeError instead of pyc.TypeError, because the
    # (successful) lookup returns a built-in int.
    with pytest.raises(TypeError):
        len(cfg['scalars.int'])

    with pytest.raises(pyc.KeyError):
        len(cfg['no-such-key'])

    # TODO enable once we support list getter/setter
    # assert 3 == len(cfg['compound']['lst_numeric'])


def test_keys():
    cfg = pyc.load_toml_str("""
        [scalars]
        flag = true
        str = 'value'

        [scalars.numeric]
        int = 1234
        flt1 = 1.0
        flt2 = -1e3

        [scalars.dates]
        day = 2023-02-12
        time = 08:30:00

        [lists]
        lst_numeric = [-42, 3, 1.5]
        """)
    assert 2 == len(cfg)
    keys = sorted(cfg.keys())
    assert 2 == len(keys)
    assert 'lists' == keys[0]
    assert 'scalars' == keys[1]

    assert 1 == len(cfg['lists'])
    keys = cfg['lists'].keys()
    assert 1 == len(keys)
    assert keys[0] == 'lst_numeric'

    assert 4 == len(cfg['scalars'])
    keys = cfg['scalars'].keys()
    assert 4 == len(keys)
    assert 'flag' in keys
    assert 'str' in keys
    assert 'numeric' in keys
    assert 'dates' in keys

    assert 3 == len(cfg['scalars']['numeric'])
    assert 3 == len(cfg['scalars.numeric'])
    tmp = cfg['scalars']['numeric'].keys()
    keys = cfg['scalars.numeric'].keys()
    assert tmp == keys
    assert 3 == len(keys)
    assert 'int' in keys
    assert 'flt1' in keys
    assert 'flt2' in keys


# def test_interator():
#     cfg = pyc.load_toml_str("""
#         [scalars]
#         flag = true
#         str = 'value'

#         [scalars.numeric]
#         int = 1234
#         flt1 = 1.0
#         flt2 = -1e3

#         [scalars.dates]
#         day = 2023-02-12
#         time = 08:30:00

#         [lists]
#         lst_numeric = [-42, 3, 1.5]
#         """)
#     cnt = sum([1 for _ in cfg])
#     assert 3 == cnt

def test_list():
    cfg = pyc.load_toml_str("""
        numbers = [1, 2, 3]
        mixed = [true, 1.5, 'value']

        nested = [
            'str',
            123,
            [1, 2, 3],
            { name = 'test', flt = 2.5}
        ]
        """)
    assert 3 == len(cfg)

    assert isinstance(cfg['numbers'], list)
    assert 3 == len(cfg['numbers'])

    # The configuration will return a *copy*. Changing it won't work:
    assert cfg['numbers'][2] == 3
    cfg['numbers'][2] = 17
    assert cfg['numbers'][2] == 3

    assert isinstance(cfg['nested'], list)

    assert isinstance(cfg['nested[0]'], str)
    assert isinstance(cfg['nested'][0], str)

    assert isinstance(cfg['nested[1]'], int)
    assert isinstance(cfg['nested'][1], int)

    assert isinstance(cfg['nested[2]'], list)
    assert isinstance(cfg['nested'][2], list)
    assert 3 == len(cfg['nested'][2])

    assert isinstance(cfg['nested[3]'], type(cfg))
    assert isinstance(cfg['nested'][3], type(cfg))
    assert 2 == len(cfg['nested'][3])
    assert 'name' in cfg['nested'][3]
    assert 'flt' in cfg['nested'][3]
