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