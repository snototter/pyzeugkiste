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

        scalar = 'value'

        [group]
        numbers = [1, 2]
        int = 123
        """)
    assert 5 == len(cfg)

    assert isinstance(cfg['numbers'], type(cfg))
    assert 3 == len(cfg['numbers'])
    assert isinstance(cfg['numbers'].list(), list)
    assert 3 == len(cfg['numbers'].list())

    # The configuration will return a view which allows changing the underlying
    # parameter.
    assert cfg['numbers'][2] == 3
    cfg['numbers'][2] = 17
    assert cfg['numbers'][2] == 17
    assert cfg['numbers[2]'] == 17

    assert isinstance(cfg['nested'], type(cfg))
    lst = cfg['nested'].list()
    assert isinstance(lst, list)
    assert 4 == len(lst)
    assert isinstance(lst[0], str)
    assert isinstance(lst[1], int)
    assert isinstance(lst[2], list)
    assert isinstance(lst[3], dict)

    assert isinstance(cfg['nested[0]'], str)
    assert isinstance(cfg['nested'][0], str)

    assert isinstance(cfg['nested[1]'], int)
    assert isinstance(cfg['nested'][1], int)

    assert isinstance(cfg['nested[2]'], type(cfg))
    assert isinstance(cfg['nested'][2], type(cfg))
    assert cfg.type('nested[2]') == pyc.ConfigType.List
    assert cfg['nested'][2].type() == pyc.ConfigType.List
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
    assert cfg.type('nested') == pyc.ConfigType.List
    assert cfg['nested'].type() == pyc.ConfigType.List
    assert len(cfg['nested']) == 0

    lst = [1, True, 3.5, 'str', datetime.date.today(), datetime.datetime.now()]
    cfg['nested'] = lst
    assert cfg.type('nested') == pyc.ConfigType.List
    assert cfg['nested'].type() == pyc.ConfigType.List
    assert lst == cfg['nested'].list()

    lst = [1, True, 'str', ['a', 'nested', 'lst']]
    cfg['nested'] = lst
    assert cfg.type('nested') == pyc.ConfigType.List
    assert cfg['nested'].type() == pyc.ConfigType.List
    assert lst == cfg['nested'].list()

    # Test a list that contains a Config instance
    another_cfg = pyc.Config()
    another_cfg['name'] = 'value'
    another_cfg['age'] = 23
    another_cfg['date'] = datetime.date.today()

    lst = [another_cfg, 1, True]
    cfg['nested'] = lst
    assert cfg.type('nested') == pyc.ConfigType.List
    assert cfg['nested'].type() == pyc.ConfigType.List
    assert cfg.type('nested[0]') == pyc.ConfigType.Group
    assert cfg['nested'][0].type() == pyc.ConfigType.Group
    # The 'list()' conversion will also convert 'another_cfg'
    # to a dict, thus we can't use the simple comparison:
    # assert lst == cfg['nested'].list()
    # But element-wise comparison works:
    for idx in range(len(lst)):
        assert lst[idx] == cfg['nested'][idx]

    # The list can also contain dictionaries
    lst = [1, 2, {"foo": "bar", "str": "value"}]
    cfg['nested'] = lst
    assert 3 == len(cfg['nested'])
    assert 1 == cfg['nested[0]']
    assert 2 == cfg['nested[1]']
    assert 2 == len(cfg['nested[2]'])
    assert 'bar' == cfg['nested[2].foo']
    assert 'value' == cfg['nested[2].str']

    # Test appending values to a list
    with pytest.raises(pyc.TypeError):
        # Can't append to the root node (which is a group)
        cfg.append('value')

    with pytest.raises(pyc.TypeError):
        # Can't append to a sub-group
        cfg['group'].append('value')

    with pytest.raises(AttributeError):
        # Can't append to a scalar value (as __getitem__ will return
        # the built-in type)
        cfg['scalar'].append(3)

    # But we can append to an existing list. Note that it also
    # works with a view/reference (i.e. "nums")
    nums = cfg['group']['numbers']
    cfg['group.numbers'].append(3.5)
    assert 3 == len(nums)
    assert pytest.approx(3.5) == nums[2]
    nums.append('str')
    assert 4 == len(nums)
    assert 4 == len(cfg['group.numbers'])
    assert 'str' == cfg['group.numbers[3]']
    assert 'str' == nums[3]

    lst = [1, '2', {'val': 3}]
    nums.append(lst)
    assert 5 == len(nums)
    assert 3 == len(nums[4])
    assert 1 == nums[4][0]
    assert '2' == nums[4][1]
    assert 3 == nums[4][2]['val']

    # 'Append' can be used to create a new list:
    assert 'new-lst' not in cfg
    cfg.append('value', key='new-lst')
    assert 'new-lst' in cfg
    assert 1 == len(cfg['new-lst'])
    assert 'value' == cfg['new-lst'][0]
    cfg['new-lst'].append(42)
    assert 2 == len(cfg['new-lst'])
    assert 42 == cfg['new-lst'][1]


def test_tuple():
    cfg = pyc.load_toml_str("""numbers = [1, 2, 3]""")

    cfg['from-tuple'] = (1, 2)
    assert 'from-tuple' in cfg
    # __getitem__ will return a reference/view
    assert isinstance(cfg['from-tuple'], type(cfg))
    # But this view can be converted to a list
    lst1 = cfg['from-tuple'].list()
    lst2 = cfg.list('from-tuple')
    assert isinstance(lst1, list)
    assert isinstance(lst2, list)
    assert lst1 == lst2

    cfg['numbers'] = (42, )
    assert len(cfg['numbers']) == 1
    assert cfg['numbers'][0] == 42

    cfg['arr'] = [[1, 2, 3], [4, 5, 6]]
    cfg['arr[0]'] = (10, 20)
    assert 2 == len(cfg['arr'][0])
    assert 2 == len(cfg['arr[0]'])
    assert 10 == cfg['arr[0][0]']
    assert 10 == cfg['arr'][0][0]
    assert 20 == cfg['arr[0][1]']
    assert 20 == cfg['arr'][0][1]

    cfg['arr'][1] = (17, )
    assert 1 == len(cfg['arr'][1])
    assert 1 == len(cfg['arr[1]'])
    assert 17 == cfg['arr[1][0]']
    assert 17 == cfg['arr'][1][0]


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

    # __getitem__ returns a reference/view. Thus, the following will
    # also change the value!
    cfg['lvl1']['lvl2']['flt'] = 4
    assert cfg['lvl1.lvl2.flt'] == pytest.approx(4)


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

    cfg = pyc.Config()
    assert cfg.empty()
    assert 0 == len(cfg)

    cfg['dict'] = pydict

    assert isinstance(cfg['dict'], type(cfg))
    assert cfg['dict'] == cfg_toml

    assert cfg['dict'].copy() == cfg_toml

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
    assert isinstance(cfg['mixed-lst'], type(cfg))
    assert cfg.type('mixed-lst') == pyc.ConfigType.List
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

    # Parameter conversion fails (pybind11) --> this will raise a standard
    # TypeError:
    with pytest.raises(TypeError):
        cfg['int'] = None

    # Parameter conversion fails (pybind11) --> this will raise a standard
    # TypeError:
    with pytest.raises(TypeError):
        cfg['str'] = None

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

    subgroup1 = cfg['scalars']
    subgroup2 = cfg['scalars.numeric']

    with pytest.raises(pyc.KeyError):
        del cfg['no-such-key']

    del cfg['str']
    assert 'str' not in cfg

    del cfg['scalars']['numeric']
    assert 'scalars.numeric' not in cfg

    with pytest.raises(pyc.KeyError):
        del cfg['scalars.numeric']

    # List elements cannot be deleted, this will raise a standard TypeError
    # due to the pybind11 lookup failure:
    with pytest.raises(TypeError):
        del cfg['lists']['lst'][2]

    with pytest.raises(TypeError):
        del cfg['lists.lst'][2]

    # If the index is part of the "key", a pyc.KeyError will be raised:
    with pytest.raises(pyc.KeyError):
        del cfg['lists.lst[2]']
    with pytest.raises(pyc.KeyError):
        del cfg['lists.lst[0]']

    # But we can delete the whole list
    del cfg['lists.lst']
    assert 'lists.lst' not in cfg

    # Caveat: deletion also affects the views/references
    copy = subgroup1.copy()
    assert 'flag' in cfg['scalars']
    assert 'flag' in subgroup1
    with pytest.raises(pyc.KeyError):
        # "scalars.numeric" has already been deleted. Thus, every
        # call on subgroup2 will raise a KeyError
        subgroup2.keys()

    del cfg['scalars']
    assert 'scalars' not in cfg
    with pytest.raises(pyc.KeyError):
        # As above, the sub-group viewed by this object no longer exists:
        subgroup1.empty()
    # But the deep copy must not be affected by the deletion:
    assert 'flag' in copy


def test_access():
    cfg = pyc.load_toml_str("""
        int = 3

        lst = [1, 2, 3]

        [[persons]]
        name = 'n1'

        [[persons]]
        name = 'n2'

        [group]
        int = 17
        lst = [4]
        nested = [[1, 2, 3], [4], 5, { name = 'n3' }]
        """)

    with pytest.raises(pyc.KeyError):
        cfg[0]

    with pytest.raises(pyc.KeyError):
        cfg['no-such-key']

    with pytest.raises(pyc.KeyError):
        cfg['group.no-such-key']

    with pytest.raises(pyc.KeyError):
        cfg['group']['no-such-key']

    with pytest.raises(pyc.KeyError):
        cfg['lst']['0']

    with pytest.raises(pyc.KeyError):
        cfg['lst'][17]

    with pytest.raises(pyc.TypeError):
        cfg[None]

    with pytest.raises(pyc.TypeError):
        cfg['lst'][None]

    # Test __getitem__
    assert 4 == len(cfg)
    assert 3 == cfg['int']
    assert 3 == len(cfg['lst'])

    assert 2 == len(cfg['persons'])
    assert isinstance(cfg['persons'], type(cfg))
    assert pyc.ConfigType.List == cfg['persons'].type()
    assert pyc.ConfigType.List == cfg.type('persons')
    assert 'n1' == cfg['persons'][0]['name']
    assert 'n1' == cfg['persons[0].name']
    assert 'n2' == cfg['persons'][1]['name']
    assert 'n2' == cfg['persons[1].name']

    assert 3 == len(cfg['group'])
    assert 1 == len(cfg['group.lst'])
    assert 1 == len(cfg['group']['lst'])

    assert 4 == len(cfg['group.nested'])
    assert 4 == len(cfg['group']['nested'])

    assert cfg['group.lst'] != cfg['group.nested[0]']
    assert cfg['group.lst'] == cfg['group.nested[1]']
    assert cfg['group']['lst'] == cfg['group.nested[1]']
    assert cfg['group']['lst'] == cfg['group']['nested[1]']
    assert cfg['group']['lst'] == cfg['group']['nested'][1]

    # Test __setitem__ with scalars
    cfg['new-str'] = ''
    assert 'new-str' in cfg
    assert pyc.ConfigType.String == cfg.type('new-str')
    assert isinstance(cfg['new-str'], str)
    assert '' == cfg['new-str']

    cfg['group.num'] = 17.5
    assert 'group.num' in cfg
    assert 'num' in cfg['group']
    assert isinstance(cfg['group.num'], float)
    assert pyc.ConfigType.FloatingPoint == cfg.type('group.num')
    assert pytest.approx(17.5) == cfg['group.num']
    assert isinstance(cfg['group']['num'], float)
    assert pyc.ConfigType.FloatingPoint == cfg['group'].type('num')
    assert pytest.approx(17.5) == cfg['group']['num']

    today = datetime.date.today()
    cfg['group']['date'] = today
    assert 'group.date' in cfg
    assert 'date' in cfg['group']
    assert isinstance(cfg['group.date'], datetime.date)
    assert pyc.ConfigType.Date == cfg.type('group.date')
    assert today == cfg['group.date']
    assert isinstance(cfg['group']['date'], datetime.date)
    assert pyc.ConfigType.Date == cfg['group'].type('date')
    assert today == cfg['group']['date']

    # Test __setitem__ on lists
    with pytest.raises(pyc.TypeError):
        cfg['lst'][1] = today  # Type cannot be changed
    cfg['lst'][1] = 42.0  # But a compatible numeric value can be used
    with pytest.raises(AttributeError):
        # __getitem__ returns the integer value (built-in type), thus
        # we can't access 'type()'
        cfg['lst'][1].type()
    assert pyc.ConfigType.Integer == cfg.type('lst[1]')
    assert 42 == cfg['lst'][1]
    assert 42 == cfg['lst[1]']

    with pytest.raises(pyc.TypeError):
        # As above, we can't change the type
        cfg['group']['lst'][0] = [1, 2, 3, 4]
    # We can, however, replace an existing list:
    assert 3 == len(cfg['group.nested[0]'])
    cfg['group']['nested'][0] = [1, 2, 3, 4]
    assert 4 == len(cfg['group']['nested'][0])
    assert 4 == len(cfg['group.nested[0]'])
    assert pyc.ConfigType.List == cfg.type('group.nested[0]')
    assert pyc.ConfigType.List == cfg['group.nested[0]'].type()
    assert pyc.ConfigType.List == cfg['group.nested'][0].type()
    assert pyc.ConfigType.List == cfg['group']['nested'][0].type()

    # Similarly, we can replace an existing list.
    # Note that both list and tuple are supported:
    cfg['lst'] = list(range(20))
    assert 20 == len(cfg['lst'])
    # ... or create a new one
    cfg['numbers'] = tuple(range(50))
    assert 50 == len(cfg['numbers'])

    # Test __setitem__ for groups
    other = pyc.Config()
    assert other.empty()
    assert 0 == len(other)
    assert other != cfg

    other['myval'] = True
    assert not other.empty()

    other['nested-config'] = cfg
    assert other != cfg
    assert other['nested-config'] == cfg
    # Ensure that the assignment *copied* the configuration
    cfg['int'] = 1234
    assert other['nested-config'] != cfg


def test_equality():
    c1 = pyc.load_toml_str("""
        int = 3

        [table1]
        int = 42
        dt = { date = 2023-04-01, time = 08:00:30 }
        lst = [1, 2, 3.4]
        nested = [[1, 2], [3], 4, { foo = 'bar', lst = [0.5, 1] }]

        [table2]
        str = 'value'
        """)
    assert c1 == c1
    assert c1['table1'] == c1['table1']
    assert c1['table1'] != c1['table2']
    assert c1['table1']['lst'] == c1['table1.lst']
    assert c1['table1']['nested'] == c1['table1.nested']
    assert c1['table1']['lst'] != c1['table1.nested']
    assert c1['table2'] == c1['table2']

    c2 = pyc.load_toml_str("""
        [group1]
        int = 42
        dt = { date = 2023-04-01, time = 08:00:30 }
        lst = [1, 2, 3.4]
        nested = [[1, 2], [3], 4, { foo = 'bar', lst = [0.5, 1] }]

        [group2]
        str = 'value'
        nested-list = [[1, 2], [3], 4, { foo = 'bar', lst = [0.5, 1] }]
        """)

    assert c1 != c2
    assert c1['table1'] == c2['group1']
    assert c1['table2'] != c2['group2']
    assert c1['table1.nested'] == c2['group2.nested-list']
    assert c1['table1.nested'] == c2['group2']['nested-list']
    assert c1['table1']['nested'] == c2['group2']['nested-list']
    assert c2['group1']['nested'] == c2['group2']['nested-list']


def test_copy():
    c1 = pyc.load_toml_str("""
        int = 3

        [group]
        int = 42
        lst = [1, 2, 3.4]
        str = 'value'
        """)
    # Deep copy of the full configuration instance:
    c2 = c1.copy()
    assert c1 == c2

    c2['int'] = 4
    assert c1 != c2

    # Deep copy of a sub-group:
    c2 = c1['group'].copy()
    assert c1 != c2
    assert c1['group'] == c2

    c2['lst'][1] = 4
    assert c1['group'] != c2

    # Deep copy of a list is not supported, because the direct child
    # parameters of a config root must be named parameters:
    with pytest.raises(pyc.TypeError):
        c2 = c1['group']['lst'].copy()


def test_clear():
    cfg = pyc.load_toml_str("""
        str = 'value'

        [group1]
        int = 42
        dt = { date = 2023-04-01, time = 08:00:30 }
        lst = [1, 2, 3.4]
        nested = [[1, 2], [3], 4, { foo = 'bar', lst = [0.5, 1] }]

        [group2]
        str = 'value'
        nested-list = [[1, 2], [3], 4, { foo = 'bar', lst = [0.5, 1] }]
        """)

    cfg['group2'].clear()
    assert cfg['group2'].empty()

    cfg['group1']['nested'].clear()
    assert cfg['group1']['nested'].empty()
    assert not cfg['group1'].empty()

    cfg.clear()
    assert cfg.empty()
