import pytest
from pyzeugkiste import config as pyc


def test_paths():
    cfg = pyc.load_toml_str("""
        int = 42
        flt = inf

        path1 = 'rel.path'
        path2 = 'rel/ative/pa.th'
        path3 = '/abs/path'

        [paths1]
        path1 = 'rel.path'
        path2 = 'rel/ative/pa.th'
        path3 = '/abs/path'
        flag = true

        [paths2]
        path1 = 'rel.path'
        path2 = 'rel/ative/pa.th'
        path3 = '/abs/path'
        str = 'value'
        """)

    copy = cfg.copy()
    assert copy == cfg

    base_pth = 'BASE'
    def check_not_changed(c):
        assert c['path1'] == 'rel.path'
        assert c['path2'] == 'rel/ative/pa.th'
        assert c['path3'] == '/abs/path'

    def check_changed(c):
        assert c['path1'] == f'{base_pth}/rel.path'
        assert c['path2'] == f'{base_pth}/rel/ative/pa.th'
        assert c['path3'] == '/abs/path'
        if 'str' in c:
            assert c['str'] == 'value'
        if 'flag' in c:
            assert c['flag']
        if 'int' in c:
            assert c['int'] == 42

    # The following wildcard will only affect the path1/path2 keys that
    # are below a sub-group:
    cfg.adjust_relative_paths(base_pth, ['*.path1', '*.path2'])
    assert copy != cfg

    check_not_changed(cfg)
    check_changed(cfg['paths1'])
    check_changed(cfg['paths2'])

    # If the name/wildcard matches a non-string parameter, it should
    # be ignored:
    cfg.adjust_relative_paths(base_pth, ['int'])
    assert cfg['int'] == 42

    cfg = copy.copy()
    cfg.adjust_relative_paths(base_pth, ['*.path*'])
    check_not_changed(cfg)
    check_changed(cfg['paths1'])
    check_changed(cfg['paths2'])


def test_names():
    cfg = pyc.load_toml_str("""
        int = 42
        hex = 0xdeadbeef
        oct = 0o755
        bin = 0b11010110

        flt1 = 3.1415
        flt2 = 5e+22
        flt3 = inf
        flt4 = nan

        str = "value"

        odt = 1979-05-27T00:32:00.999999-07:00

        lst = [1, 2, 3, { int = 17 }]

        [group]
        flag = true
        time = 07:30:00
        """)

    assert cfg.keys() == cfg.list_parameter_names(recursive=False)

    keys = cfg.keys()
    keys.extend(['lst[3].int', 'group.flag', 'group.time'])
    assert sorted(keys) == sorted(cfg.list_parameter_names(recursive=True))

    keys.extend(['lst[0]', 'lst[1]', 'lst[2]', 'lst[3]'])
    assert sorted(keys) == sorted(cfg.list_parameter_names(include_array_entries=True))


def test_placeholders():
    cfg = pyc.load_toml_str("""
        int = 3
        flt = inf
        str1 = '%REP%!'
        str2 = 'value%REP%'
        str3 = '$TOKEN'

        [tbl1]
        str1 = '%REP%!'
        str2 = 'value%REP%'
        str3 = '$TOKEN'

        [tbl2]
        str1 = '%REP%!'
        str2 = 'value%REP%'
        str3 = '$TOKEN'

        [tbl2.tbl]
        str1 = '%REP%!'
        str2 = 'value%REP%'
        str3 = '$TOKEN'
        """)

    copy = cfg.copy()
    assert copy == cfg

    with pytest.raises(pyc.ValueError):
        # Search string cannot be empty
        cfg.replace_placeholders([('', '...')])

    cfg.replace_placeholders([('%REP%', '...')])
    assert copy != cfg


    def check_not_changed(c):
        assert c['str1'] == '%REP%!'
        assert c['str2'] == 'value%REP%'
        assert c['str3'] == '$TOKEN'

    def check_changed(c):
        assert c['str1'] == '...!'
        assert c['str2'] == 'value...'
        assert c['str3'] == '$TOKEN'

    # By default, *all* occurences are replaced:
    check_changed(cfg)
    check_changed(cfg['tbl1'])
    check_changed(cfg['tbl2'])
    check_changed(cfg['tbl2.tbl'])

    # Now, only replace tokens in a sub-group:
    cfg = copy.copy()
    another_copy = copy.copy()

    cfg.replace_placeholders([('%REP%', '...')], key='tbl2')
    assert another_copy != cfg
    another_copy['tbl2'].replace_placeholders([('%REP%', '...')])
    assert another_copy == cfg

    check_not_changed(cfg)
    check_not_changed(cfg['tbl1'])
    check_changed(cfg['tbl2'])
    check_changed(cfg['tbl2.tbl'])

    # Replace multiple tokens at once
    cfg['tbl2']['tbl'].replace_placeholders([
        ('%REP%', '...'), ('$TOKEN', '')])

    cfg['tbl2.tbl.str1'] = '...!'
    cfg['tbl2.tbl.str2'] = 'value...'
    cfg['tbl2.tbl.str3'] = ''

    # Caveat: replacements will be performed in order. This test shows
    # the side effect of using a bad search string:
    another_copy['tbl2']['tbl'].replace_placeholders([
        ('%REP%', '...'), ('$TOKEN', ''), ('.', '__')])

    another_copy['tbl2.tbl.str1'] = '______!'
    another_copy['tbl2.tbl.str2'] = 'value______'
    another_copy['tbl2.tbl.str3'] = ''
