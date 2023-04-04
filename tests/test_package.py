import pytest
import pyzeugkiste


def test_attributes():
    assert hasattr(pyzeugkiste, "__doc__")
    assert isinstance(pyzeugkiste.__doc__, str)
    assert hasattr(pyzeugkiste, "__version__")
    assert isinstance(pyzeugkiste.__version__, str)
    assert hasattr(pyzeugkiste, "__werkzeugkiste_version__")
    assert isinstance(pyzeugkiste.__werkzeugkiste_version__, str)


def test_submodules():
    # Example call/instantiation for 'stuff' from each submodule to ensure
    # that it can be loaded properly.

    assert 'args' in pyzeugkiste.__all__
    assert 'config' in pyzeugkiste.__all__
    assert 'files' in pyzeugkiste.__all__  # TODO: drop file utils
    assert 'geo' in pyzeugkiste.__all__
    assert 'strings' in pyzeugkiste.__all__

    assert hasattr(pyzeugkiste.args, 'ValidateDate')
    x = pyzeugkiste.config.Configuration()
    x = pyzeugkiste.geo.Vec2d()
    assert 3 == pyzeugkiste.strings.levenshtein_distance('to', 'from')
