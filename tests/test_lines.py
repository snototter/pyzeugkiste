import pytest
import pyzeugkiste  # Needed for the deserialization checks, i.e. eval(repr(x))
from pyzeugkiste import geo as pg
import math
import pickle
import numpy as np


def equals(line1, line2):
    # Since a "LineXd" instance can represent both a line and a segment,
    # it does not provide an overload for the comparison operator.
    return (line1.pt1 == line2.pt1) and (line1.pt2 == line2.pt2)


def generic_line_test_helper(line):
    restored = eval(repr(line))
    assert equals(line, restored)
    # TODO: add checks which should work for both 2D & 3D lines


def test_line2d():
    line = pg.Line2d((0, 0), (0, 0))
    assert not line.is_valid()
    assert equals(line, eval(repr(line)))

    line = pg.Line2d((0, 1), (1, 0))
    generic_line_test_helper(line)

# TODO: add tests for 3D line
