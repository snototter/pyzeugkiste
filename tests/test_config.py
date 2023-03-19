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

def test_str():
    cfg = pyc.load_toml_str("""
        flag = true
        num = 1.5
        str = 'value'
        """)
    assert len(cfg) == 3

    # Try to load as an invalid/incompatible type --> TypeError
    with pytest.raises(pyc.TypeError):
        cfg.get_str('flag')
    with pytest.raises(TypeError):
        cfg.get_str('flag')

    with pytest.raises(pyc.TypeError):
        cfg.get_str('num')
    with pytest.raises(TypeError):
        cfg.get_str('num')

    with pytest.raises(pyc.TypeError):
        cfg.get_bool('str')

    with pytest.raises(pyc.TypeError):
        cfg.get_int('str')

    # Retrieve value as intended:
    assert 'value' == cfg['str']
    assert 'value' == cfg.get_str('str')
    assert isinstance(cfg['str'], str)
    assert isinstance(cfg.get_str('str'), str)

    # Use default values if the key is not found:
    assert 'A' == cfg.get_str_or('unknown', 'A')

    # ... but an invalid type remains an invalid type:
    with pytest.raises(pyc.TypeError):
        cfg.get_str_or('flag', 'A')

    # Changing a value is allowed
    cfg['str'] = 'changed'
    assert 'changed' == cfg['str']

    # But changing the type is not allowed
    with pytest.raises(pyc.TypeError):
        cfg['str'] = True

    with pytest.raises(pyc.TypeError):
        cfg.set_str('num', '123')


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
    assert cfg.get_time('time') == tm
    assert isinstance(cfg['time'], datetime.time)

    with pytest.raises(pyc.KeyError):
        cfg.get_time('no-such-key')

    with pytest.raises(pyc.TypeError):
        cfg.get_date('time')

    with pytest.raises(pyc.TypeError):
        cfg.get_datetime('time')

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

    # Lists cannot be replaced by scalars
    with pytest.raises(pyc.TypeError):
        cfg['nested'] = 3

    with pytest.raises(pyc.TypeError):
        cfg['nested'] = 'a string'

    # Lists can be replaced by any other list
    cfg['nested'] = []
    assert len(cfg['nested']) == 0

    lst = [1, True, 3.5, 'str', datetime.date.today(), datetime.datetime.now()]
    cfg['nested'] = lst
    assert lst == cfg['nested']

    lst = [1, True, 'str', ['a', 'nested', 'lst']]
    cfg['nested'] = lst
    assert lst == cfg['nested']

    # The list can also contain Configuration instances
    another_cfg = pyc.Configuration()
    another_cfg['name'] = 'value'
    another_cfg['age'] = 23
    another_cfg['date'] = datetime.date.today()

    lst = [another_cfg, 1, True]
    cfg['nested'] = lst
    assert lst == cfg['nested']

    # The list can also contain dictionaries
    lst = [1, 2, {"foo": "bar", "str": "value"}]
    cfg['nested'] = lst
    assert 3 == len(cfg['nested'])
    assert 1 == cfg['nested[0]']
    assert 2 == cfg['nested[1]']
    assert 2 == len(cfg['nested[2]'])
    assert 'bar' == cfg['nested[2].foo']
    assert 'value' == cfg['nested[2].str']


def test_tuple():
    cfg = pyc.load_toml_str("""numbers = [1, 2, 3]""")

    cfg['from-tuple'] = (1, 2)
    assert 'from-tuple' in cfg
    assert isinstance(cfg['from-tuple'], list)

    cfg['numbers'] = (42, )
    assert len(cfg['numbers']) == 1
    assert cfg['numbers'][0] == 42


def test_group():
    cfg = pyc.load_toml_str("""
        numbers = [1, 2, 3]
        
        [scalars]
        int = 3
        str = 'value'

        [lvl1.lvl2]
        flt = 3.5
        """)
    assert 3 == len(cfg)

    assert isinstance(cfg['scalars'], type(cfg))
    assert isinstance(cfg['lvl1.lvl2'], type(cfg))
    assert isinstance(cfg['lvl1']['lvl2'], type(cfg))

    with pytest.raises(pyc.TypeError):
        cfg['lvl1.lvl2'] = False
    
    with pytest.raises(pyc.TypeError):
        cfg['lvl1.lvl2.flt'] = 'value'
    
    cfg['lvl1.lvl2.flt'] = 2
    assert cfg['lvl1.lvl2.flt'] == pytest.approx(2)
    # But it should still be a floating point parameter
    assert isinstance(cfg['lvl1.lvl2.flt'], float)

    # __getitem__ returns a copy. Thus, the following will only change
    # a temporary!
    cfg['lvl1']['lvl2']['flt'] = 4
    assert cfg['lvl1.lvl2.flt'] == pytest.approx(2)


def test_dict_set():
    cfg_toml = pyc.load_toml_str("""
        int = 1
        
        [lvl1]
        str = 'value'

        [lvl1.lvl2]
        flt = 3.5
        numbers = [1, 2, 3]
        nested = [{ name = 'foo' }, { name = 'bar', str = 'value' }]
        """)
    
    pydict = {
        "int": 1,
        "lvl1": {
            "str": "value",
            "lvl2": {
                "flt": 3.5,
                "numbers": [1, 2, 3],
                "nested": [
                    { "name": "foo" },
                    { "name": "bar", "str": "value" }
                ]
            }
        }
    }

    cfg = pyc.Configuration()
    cfg['dict'] = pydict

    assert isinstance(cfg['dict'], type(cfg))
    cfg = cfg['dict']
    assert cfg == cfg_toml

    tmp = cfg.to_dict()
    assert tmp == pydict

    assert 2 == len(cfg)
    assert 1 == cfg['int']
    assert 2 == len(cfg['lvl1'])
    assert 'value' == cfg['lvl1.str']

    assert 'value' == cfg['lvl1.lvl2.nested[1].str']
    assert 'value' == cfg['lvl1']['lvl2']['nested'][1]['str']

    lst = [1, 2, {"foo": 3, "3": "value"}]
    cfg['mixed-lst'] = lst
    assert isinstance(cfg['mixed-lst'], list)
    assert 3 == len(cfg['mixed-lst'])
    assert 1 == cfg['mixed-lst[0]']
    assert 2 == cfg['mixed-lst[1]']
    assert isinstance(cfg['mixed-lst[2]'], type(cfg))
    assert 2 == len(cfg['mixed-lst[2]'])
    assert 3 == cfg['mixed-lst[2].foo']
    assert 'value' == cfg['mixed-lst[2].3']

    # Dictionary keys must be strings:
    lst = [1, 2, {"foo": "bar", 3: "value"}]
    with pytest.raises(pyc.TypeError):
        cfg['mixed-lst'] = lst
    
    # Dictionary keys must also be bare keys:
    lst = [1, 2, {"fo o": "bar", "3": "value"}]
    with pytest.raises(pyc.TypeError):
        cfg['mixed-lst'] = lst

    # Valid dictionary keys must not include dots, as these are reserved for
    # a "key path".
    lst = [1, 2, {"foo.invalid": "bar", "3": "value"}]
    with pytest.raises(pyc.TypeError):
        cfg['mixed-lst'] = lst


def test_dict_get():
    cfg = pyc.load_toml_str("""
        [scalars]
        int = 1
        flt = 1e-17
        flag = true
        str = 'value'

        [lists]
        nums = [1, 2, 3.123]
        mixed = [true, 42, 'value']
        nested = [
            [1, 2, 3],
            17,
            { str = 'value' }
        ]

        [tables]
        t1 = { name = 'value' }
        t2 = { name = 'foo', t2-1 = { name = 'bar' }}

        [dates]
        day = 2023-03-19
        time = 19:48:00
        dt = 2023-03-19T19:48:00+01:00
        """)
    d = cfg.to_dict()
    assert len(d) == 4
    assert 'scalars' in d
    assert isinstance(d['scalars'], dict)
    assert isinstance(d['scalars']['int'], int)
    assert isinstance(d['scalars']['flt'], float)
    assert isinstance(d['scalars']['flag'], bool)
    assert isinstance(d['scalars']['str'], str)

    assert 'lists' in d
    assert isinstance(d['lists'], dict)
    assert isinstance(d['lists']['nums'], list)
    assert [1, 2, 3.123] == d['lists']['nums']

    assert isinstance(d['lists']['mixed'], list)
    assert [True, 42, 'value'] == d['lists']['mixed']

    assert isinstance(d['lists']['nested'], list)
    assert isinstance(d['lists']['nested'][0], list)
    assert [1, 2, 3] == d['lists']['nested'][0]
    assert isinstance(d['lists']['nested'][1], int)
    assert isinstance(d['lists']['nested'][2], dict)

    assert 'tables' in d
    assert isinstance(d['tables'], dict)
    assert isinstance(d['tables']['t1'], dict)
    assert isinstance(d['tables']['t1']['name'], str)
    assert isinstance(d['tables']['t2'], dict)
    assert isinstance(d['tables']['t2']['name'], str)
    assert isinstance(d['tables']['t2']['t2-1'], dict)
    assert isinstance(d['tables']['t2']['t2-1']['name'], str)

    assert 'dates' in d
    assert isinstance(d['dates'], dict)
    assert isinstance(d['dates']['day'], datetime.date)
    assert isinstance(d['dates']['time'], datetime.time)
    assert isinstance(d['dates']['dt'], datetime.datetime)
    assert pytest.approx(3600) == d['dates']['dt'].utcoffset().total_seconds()


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
    assert 3 == len(cfg['compound.lst_numeric'])
    assert 3 == len(cfg['compound']['lst_numeric'])

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

    # Only bare keys (alphanumeric + '-'/'_') are supported
    with pytest.raises(pyc.KeyError):
        cfg[' invalid '] = 3

    with pytest.raises(pyc.KeyError):
        cfg[' inv.alid '] = 3
    
    with pytest.raises(pyc.KeyError):
        cfg['inv.alid '] = 3

    with pytest.raises(pyc.KeyError):
        cfg['inv. alid'] = 3
    
    with pytest.raises(pyc.KeyError):
        cfg['inv.al id'] = 3

    with pytest.raises(pyc.KeyError):
        cfg['inv alid'] = 3
    
    cfg['valid'] = 3
    assert 'valid' in cfg
    assert 3 == cfg['valid']


def test_none():
    cfg = pyc.load_toml_str("""
        flag = true
        str = 'value'
        int = 1234

        lst = [1, 2, 3]
        """)
    assert 4 == len(cfg)

    with pytest.raises(pyc.TypeError):
        cfg['flag'] = None
    
    # Note, however, that None will be converted by pybind11 to 'False' if
    # used as method parameter:
    cfg.set_bool('flag', None)
    assert not cfg['flag']
    
    # Parameter conversion fails (pybind11) --> this will raise a standard
    # TypeError:
    with pytest.raises(TypeError):
        cfg.set_int('int', None)
    
    # Parameter conversion fails (pybind11) --> this will raise a standard
    # TypeError:
    with pytest.raises(TypeError):
        cfg.set_str('str', None)

    with pytest.raises(pyc.TypeError):
        cfg['lst[0]'] = None

    with pytest.raises(pyc.TypeError):
        cfg['none-lst'] = [1, 2, None]
    
    with pytest.raises(pyc.TypeError):
        cfg['tbl'] = { "param": 1, "another": None }


def test_delete():
    cfg = pyc.load_toml_str("""
        str = 'value'

        [scalars]
        flag = true
        str = 'value'

        [scalars.numeric]
        int = 1234
        flt1 = 1.0
        flt2 = -1e3

        [lists]
        lst = [-42, 3, 1.5]
        """)
    
    with pytest.raises(pyc.KeyError):
        del cfg['no-such-key']

    del cfg['str']
    assert 'str' not in cfg

    # Will remove the group only from the COPY, not the original configuration!
    del cfg['scalars']['numeric']
    assert 'scalars.numeric' in cfg
    del cfg['scalars.numeric']
    assert 'scalars.numeric' not in cfg

    # Similarly, lists are also returned by-value (i.e. we're deleting from
    # a copy)
    del cfg['lists']['lst'][2]
    assert len(cfg['lists']['lst']) == 3

    del cfg['lists.lst'][2]
    assert len(cfg['lists.lst']) == 3

    # Deleting a single element from a list is not allowed
    with pytest.raises(pyc.KeyError):
        del cfg['lists.lst[2]']
    with pytest.raises(pyc.KeyError):
        del cfg['lists.lst[0]']
    
    # But we can delete the whole list
    del cfg['lists.lst']
    assert 'lists.lst' not in cfg
