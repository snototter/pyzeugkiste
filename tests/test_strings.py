import pytest
from pyzeugkiste import strings as pst


def test_levenshtein():
    with pytest.raises(TypeError):
        pst.levenshtein_distance('test', None)
    
    with pytest.raises(TypeError):
        pst.levenshtein_distance(None, None)

    with pytest.raises(TypeError):
        pst.levenshtein_distance(None, 'test')

    assert 0 == pst.levenshtein_distance('', '')
    assert 0 == pst.levenshtein_distance('FrobMorten', 'FrobMorten')
    assert 1 == pst.levenshtein_distance('FrobMorten', 'Frobmorten')
    assert 2 == pst.levenshtein_distance('FrobMorten', 'frobmorten')

    assert 7 == pst.levenshtein_distance('frambozzle', 'frobmorten')
    assert 7 == pst.levenshtein_distance('frobmorten', 'frambozzle')

    assert 3 == pst.levenshtein_distance('kitten', 'sitting')
    assert 3 == pst.levenshtein_distance('Hello', 'halo')

    assert 6 == pst.levenshtein_distance('my-key', '')


def test_shorten():
    # Edge cases: empty & desired length 0 or longer than string
    assert '' == pst.shorten('', 4)
    assert 'abc' == pst.shorten('abc', 3)
    assert 'abc' == pst.shorten('abc', 10)

    # Desired length shorter than (custom) ellipsis
    with pytest.raises(ValueError):
        pst.shorten('abc', 2)
    
    with pytest.raises(ValueError):
        pst.shorten('0123456789', length=3, ellipsis="abcd")

    # Ellipsis left
    assert '...' == pst.shorten('0123456789', 3, pos=-1)
    assert '...9' == pst.shorten('0123456789', 4, pos=-1)
    assert '...89' == pst.shorten('0123456789', 5, pos=-1)
    assert '789' == pst.shorten('0123456789', 3, pos=-1, ellipsis='')
    assert '_789' == pst.shorten('0123456789', 4, pos=-1, ellipsis='_')
    assert '_6789' == pst.shorten('0123456789', 5, pos=-1, ellipsis='_')

    # Ellipsis centered
    assert '...' == pst.shorten('0123456789', 3, pos=0)
    assert '...9' == pst.shorten('0123456789', 4, pos=0)
    assert '0...9' == pst.shorten('0123456789', 5, pos=0)
    assert '_' == pst.shorten('0123456789', 1, pos=0, ellipsis='_')
    assert '_9' == pst.shorten('0123456789', 2, pos=0, ellipsis='_')
    assert '0_9' == pst.shorten('0123456789', 3, pos=0, ellipsis='_')
    assert '089' == pst.shorten('0123456789', 3, pos=0, ellipsis='')
    assert '0_89' == pst.shorten('0123456789', 4, pos=0, ellipsis='_')
    assert '01_89' == pst.shorten('0123456789', 5, pos=0, ellipsis='_')

    # Ellipsis right
    assert '...' == pst.shorten('0123456789', 3, pos=+1)
    assert '0...' == pst.shorten('0123456789', 4, pos=+1)
    assert '01...' == pst.shorten('0123456789', 5, pos=+1)
    assert '012' == pst.shorten('0123456789', 3, pos=+1, ellipsis='')
    assert '012_' == pst.shorten('0123456789', 4, pos=+1, ellipsis='_')
    assert '0123_' == pst.shorten('0123456789', 5, pos=+1, ellipsis='_')


def test_slugify():
    with pytest.raises(TypeError):
        pst.slugify(None)

    assert '' == pst.slugify('')

    s = 'nothing-to-be-slugged'
    assert s == pst.slugify(s)
    assert s.replace('-', '') == pst.slugify(s, strip_dashes=True)

    assert 'replace-some-spaces-and-underscores' == pst.slugify(' replace:\tsome_spaces  and UNDERSCORES  _- ')
    assert '' == pst.slugify(' \r\n\t\v\f')
    assert 'a' == pst.slugify('a \r\n\t\v\f')
    assert 'b' == pst.slugify(' \r\n\t\v\fb')
    assert 'a-b' == pst.slugify('A \r\n\t\v\fB')

    assert 'nr2-pm23pc' == pst.slugify('#2 \u00b123%')
    assert '' == pst.slugify(':?`!')

    assert 'oesterreich' == pst.slugify('Österreich!')
    assert 'euro-dollar-mu-c' == pst.slugify('€   $ \t \n µ   \t\u00a9')
    assert 'aeaeoeoeueue' == pst.slugify('ÄäÖöÜü')