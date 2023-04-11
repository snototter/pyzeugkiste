import pytest
import numpy as np
import datetime
from pyzeugkiste import config as pyc

def test_exception_hierarchy():
    assert issubclass(pyc.KeyError, KeyError)
    assert issubclass(pyc.TypeError, TypeError)
    assert issubclass(pyc.ParseError, RuntimeError)
    assert issubclass(pyc.ValueError, ValueError)


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
    # The 'list()' conversion will convert 'another_cfg'
    # to a dict. The comparison operator, however, allows
    # testing a dict against a Config:
    assert lst == cfg['nested'].list()
    assert isinstance(cfg['nested'][0], type(cfg))
    assert isinstance(cfg['nested'].list()[0], dict)
    # We can also perform element-wise comparison:
    for idx in range(len(lst)):
        assert lst[idx] == cfg['nested'][idx]
    # Try the same via __iter__
    for idx, val in enumerate(cfg['nested']):
        assert lst[idx] == val

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
    # A Config can be compared against a dict
    assert cfg == tmp
    assert tmp == cfg
    tmp['foo'] = 'bar'
    assert tmp != cfg

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


def test_parameter_len():
    cfg = pyc.load_toml_str("""
        str = 'value'
        int = 3

        [values]
        str = 'hello'
        int = 42
        flt = 1e-3
        arr = [1, 2, 3]
        """)

    assert len(cfg) == 3
    assert cfg.parameter_len('') == 3

    assert len(cfg['values']) == 4
    assert cfg.parameter_len('values') == 4

    assert len(cfg['values']['arr']) == 3
    assert cfg.parameter_len('values.arr') == 3


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


def test_container_interface():
    cfg = pyc.load_toml_str("""
        str = 'value'

        [group]
        int = 42
        dt = { date = 2023-04-01, time = 08:00:30 }
        lst = [1, 2, 3.4]
        nested = [[1, 2], [3], 4, { foo = 'bar', lst = [0.5, 1] }]
        """)

    #### Root element is a group --> map-like
    keys_man = cfg.keys()
    # The current implementation does not support "dictionary views", i.e.
    # 'keys' returns a snapshot that will not be updated automatically
    cfg['flag'] = True
    assert keys_man != cfg.keys()

    # __iter__ iterates over the keys
    keys_it = [k for k in cfg]
    keys_man = cfg.keys()
    assert keys_it == keys_man
    # list() uses __iter__:
    assert keys_man == list(cfg)

    #### Subgroups are also map-like
    # __iter__ iterates over the keys
    keys_it = [k for k in cfg['group']]
    keys_man = cfg['group'].keys()
    assert keys_it == keys_man

    values_man = cfg['group'].values()
    for idx, kv in enumerate(cfg['group'].items()):
        assert keys_man[idx] == kv[0]
        assert values_man[idx] == kv[1]


    #### Lists are sequence-like
    # __iter__ iterates over the values
    values_it = [value for value in cfg['group']['lst']]
    values_man = cfg['group']['lst'].list()
    assert values_it == values_man
    assert isinstance(values_it[0], int)
    assert isinstance(values_it[1], int)
    assert isinstance(values_it[2], float)

    # Iterator & 'list()' will convert nested list/group (views) to
    # list/dict
    values_it = [value for value in cfg['group']['nested']]
    values_man = cfg['group']['nested'].list()
    assert values_it == values_man
    assert isinstance(values_it[0], list)
    assert isinstance(values_it[1], list)
    assert isinstance(values_it[2], int)
    assert isinstance(values_it[3], dict)
    assert isinstance(values_it[3]['foo'], str)
    assert isinstance(values_it[3]['lst'], list)

    # A sequence-like object doesn't support 'keys', etc.
    with pytest.raises(pyc.TypeError):
        cfg['group.lst'].keys()

    with pytest.raises(pyc.TypeError):
        cfg['group.lst'].values()

    with pytest.raises(pyc.TypeError):
        cfg['group.lst'].items()


def test_get_numpy():
    cfg = pyc.load_toml_str("""
        camera-matrix = [
            [800,   0, 400],
            [  0, 750, 300],
            [  0,   0,   1]
        ]

        lst-flt = [0.0, -3.5, 1e6]
        
        empty = []

        mat-uint8 = [
            [0, 127],
            [10, 100],
            [32, 64]
        ]
        """)
    
    ###########################################################################
    # Intended use case: load a nested list as matrix of a desired dtype,
    # e.g. double-precision floating point:
    mat = cfg['camera-matrix'].numpy(dtype=np.float64)
    assert mat.ndim == 2
    assert mat.dtype == np.float64
    assert (mat.shape[0] == 3) and (mat.shape[1] == 3)
    assert mat.flags.c_contiguous
    assert pytest.approx(800) == mat[0, 0]
    assert pytest.approx(0) == mat[0, 1]
    assert pytest.approx(400) == mat[0, 2]
    assert pytest.approx(0) == mat[1, 0]
    assert pytest.approx(750) == mat[1, 1]
    assert pytest.approx(300) == mat[1, 2]
    assert pytest.approx(0) == mat[2, 0]
    assert pytest.approx(0) == mat[2, 1]
    assert pytest.approx(1) == mat[2, 2]

    # Test default parameters:
    default = cfg['camera-matrix'].numpy()
    assert np.array_equal(mat, default)

    # Alternatively, we can look it up via:
    tmp = cfg.numpy(key='camera-matrix', dtype=np.float64)
    assert np.allclose(tmp, mat)
    
    # Keep a copy for later validation checks
    m3x3 = mat.copy()
    m2x3 = mat[:2, :]
    assert m2x3.shape[0] == 2

    ###########################################################################
    # Store matrix as configuration parameter:
    assert m2x3.flags.c_contiguous
    cfg['m2x3'] = m2x3
    assert cfg.type('m2x3') == pyc.ConfigType.List
    assert 2 == len(cfg['m2x3'])
    assert cfg.type('m2x3[0]') == pyc.ConfigType.List
    assert 3 == len(cfg['m2x3'][0])
    assert 3 == len(cfg['m2x3'][1])

    ###########################################################################
    # Set matrices with different storage orders (default is row-major)
    mat_c = np.array([[1,2,3], [4,6,7]], order='C')
    mat_f = np.array([[1,2,3], [4,6,7]], order='F')
    assert np.array_equiv(mat_c, mat_f)
    assert mat_c.flags.c_contiguous
    assert not mat_c.flags.f_contiguous
    assert not mat_f.flags.c_contiguous
    assert mat_f.flags.f_contiguous
    cfg['row-major'] = mat_c
    cfg['col-major'] = mat_f
    
    ret_c = cfg['row-major'].numpy(dtype=np.float64)
    assert ret_c.flags.c_contiguous
    assert np.array_equiv(mat_c, ret_c)
    ret_c = cfg['row-major'].numpy(dtype=mat_c.dtype)
    assert ret_c.flags.c_contiguous
    assert np.array_equiv(mat_c, ret_c)

    ret_f = cfg['col-major'].numpy(dtype=np.float64)
    assert ret_f.flags.c_contiguous  # Returned matrix is always C-contiguous
    assert np.array_equiv(mat_c, ret_f)
    ret_f = cfg['col-major'].numpy(dtype=mat_f.dtype)
    assert ret_f.flags.c_contiguous
    assert np.array_equiv(mat_c, ret_f)

    ###########################################################################
    # Load matrices as single-precision float
    mat = cfg['camera-matrix'].numpy(dtype=np.float32)
    assert mat.dtype == np.float32
    assert mat.flags.c_contiguous
    assert np.array_equal(m3x3.astype(np.float32), mat)

    mat = cfg['m2x3'].numpy(dtype=np.float32)
    assert mat.dtype == np.float32
    assert mat.flags.c_contiguous
    assert np.array_equal(m2x3.astype(np.float32), mat)

    ###########################################################################
    # Load matrices as 64-bit integer
    mat = cfg['camera-matrix'].numpy(dtype=np.int64)
    assert mat.dtype == np.int64
    assert mat.flags.c_contiguous
    assert np.array_equal(m3x3.astype(np.int64), mat)

    mat = cfg['m2x3'].numpy(dtype=np.int64)
    assert mat.dtype == np.int64
    assert mat.flags.c_contiguous
    assert np.array_equal(m2x3.astype(np.int64), mat)

    ###########################################################################
    # Load matrices as 32-bit integer
    mat = cfg['camera-matrix'].numpy(dtype=np.int32)
    assert mat.dtype == np.int32
    assert mat.flags.c_contiguous
    assert np.array_equal(m3x3.astype(np.int32), mat)

    mat = cfg['m2x3'].numpy(dtype=np.int32)
    assert mat.dtype == np.int32
    assert mat.flags.c_contiguous
    assert np.array_equal(m2x3.astype(np.int32), mat)

    ###########################################################################
    # Load matrices as 8-bit unsigned integer
    with pytest.raises(pyc.TypeError):
        # Values exceed uint8 range
        mat = cfg['camera-matrix'].numpy(dtype=np.uint8)
    
    mat = cfg['mat-uint8'].numpy(dtype=np.uint8)
    assert mat.dtype == np.uint8
    assert mat.flags.c_contiguous
    assert 3 == mat.shape[0]
    assert 2 == mat.shape[1]
    assert 0 == mat[0, 0]
    assert 127 == mat[0, 1]
    assert 10 == mat[1, 0]
    assert 100 == mat[1, 1]
    assert 32 == mat[2, 0]
    assert 64 == mat[2, 1]

    ###########################################################################
    # Other types are not supported (and will not be added, unless there is
    # a specific use case)
    with pytest.raises(pyc.TypeError):
        cfg['camera-matrix'].numpy(dtype=np.int8)

    with pytest.raises(pyc.TypeError):
        cfg['camera-matrix'].numpy(dtype=np.int16)

    with pytest.raises(pyc.TypeError):
        cfg['camera-matrix'].numpy(dtype=np.uint16)


    ###########################################################################
    # Edge case empty matrix:
    mat = cfg['empty'].numpy(dtype=np.uint8)
    assert mat.dtype == np.uint8
    assert mat.flags.c_contiguous
    assert 0 == len(mat)

    ###########################################################################
    # Test optional matrices
    mat = cfg.numpy_or('mat-uint8', dtype=np.int32, value=None)
    assert mat.dtype == np.int32
    assert mat.shape == (3, 2)

    mat = cfg.numpy_or('unknown', dtype=np.int32, value=None)
    assert mat is None

    # The _or getter can't be invoked without a key, because the __getitem__
    # lookup would already fail:
    with pytest.raises(KeyError):
        cfg['unknown'].numpy_or(dtype=np.int32, value=None)

    # TODO doc how to extend it
    # 1) wzk: Add GetMatrixT to werkzeugkiste::config::Configuration
    # 2) wzk: Add test cases to compound test
    # 3) pzk: Extend `Config::GetMatrix`
    # 4) pzk: Add test cases to test_get_numpy()



def test_set_from_numpy():
    cfg = pyc.Config()

    ## Empty numpy array
    # Set
    mat = np.array([], dtype=np.int64)
    assert 0 == len(mat)
    cfg['empty'] = mat
    assert 0 == len(cfg['empty'])
    # Get
    ret = cfg['empty'].numpy(dtype=np.int64)
    assert 0 == len(ret)

    ## 2D matrix
    # Set
    mat = np.random.rand(5, 3)
    cfg['rand'] = mat
    # Get
    ret = cfg['rand'].numpy(dtype=np.float64)
    assert np.allclose(ret, mat)
    # The values should be in [0, 1], thus not representable by an int:
    with pytest.raises(pyc.TypeError):
        cfg['rand'].numpy(dtype=np.int32)

    ## 3D matrix is not supported
    with pytest.raises(ValueError):
        cfg['chan3'] = np.zeros((5, 4, 3), dtype=np.int32)

    ## Boolean arrays are supported
    cfg['bools'] = np.array([True, False, True], dtype=bool)
    assert 'bools' in cfg
    assert 3 == len(cfg['bools'])
    assert [True, False, True] == cfg['bools'].list()

    ## 1D matrix Nx1
    arr = np.array([1, 2, 3, 4], dtype=np.int32).reshape((4, 1))
    assert arr.shape == (4, 1)
    cfg['arr-1d'] = arr
    assert 'arr-1d' in cfg
    assert 4 == len(cfg['arr-1d'])
    assert [1, 2, 3, 4] == cfg['arr-1d'].list()
    mat = cfg['arr-1d'].numpy(dtype=np.int32)
    assert mat.shape == (4, 1)
    
    ## 1D matrix 1xN
    arr = np.array([1, 2], dtype=np.int32).reshape((1, 2))
    assert arr.shape == (1, 2)
    cfg['arr-1d'] = arr
    assert 'arr-1d' in cfg
    assert 2 == len(cfg['arr-1d'])
    assert [1, 2] == cfg['arr-1d'].list()
    mat = cfg['arr-1d'].numpy(dtype=np.int32)
    assert mat.shape == (2, 1)

    ## Vector (N,)
    # Will be stored as single list and retrieved as a Nx1 matrix:
    arr = np.array([1, 2, 3], dtype=np.int32)
    assert arr.shape == (3,)
    cfg['arr-1d'] = arr
    assert 'arr-1d' in cfg
    assert 3 == len(cfg['arr-1d'])
    assert [1, 2, 3] == cfg['arr-1d'].list()
    mat = cfg['arr-1d'].numpy(dtype=np.int32)
    assert mat.shape == (3, 1)

    