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


def approx(val):
    # Due to rounding & precision loss, we typically have deviations around
    # 1e-5 or e-6 for the angles. To prevent unnecessarily flaky tests,
    # reduce the desired precision to 1e-3:
    return pytest.approx(val, abs=1e-3)


def generic_line_test_helper(line):
    restored = eval(repr(line))
    assert equals(line, restored)
    # TODO: add checks which should work for both 2D & 3D lines

    assert approx(0) == line.angle_vec_deg(line.direction())
    assert approx(180) == line.angle_vec_deg(-line.direction())
    assert approx(math.pi) == line.angle_vec_rad(-line.direction())

    assert approx(0) == line.angle_deg(line)
    assert approx(180) == line.angle_deg(line.reversed())
    assert approx(math.pi) == line.angle_rad(line.reversed())


def test_line2d():
    line = pg.Line2d((0, 0), (0, 0))
    assert not line.is_valid()
    assert equals(line, eval(repr(line)))

    line = pg.Line2d((0, 1), (1, 0))
    generic_line_test_helper(line)

    ## Line angles
    l1 = pg.Line2d((0.0, 0.0), (3.0, 0.0))
    l2 = pg.Line2d((1.0, -0.6), (1.0, -0.2))
    assert not l1.is_collinear(l2)
    assert not l1.is_parallel(l2)
    assert approx(90) == l1.angle_deg(l2)
    assert approx(90) == l1.angle_deg(l2.reversed())
    assert approx(math.pi / 2) == l1.angle_rad(l2)
  
    l2 = pg.Line2d((0, 0), (1, 1))
    assert not l1.is_collinear(l2)
    assert not l1.is_parallel(l2)
    assert approx(45) == l1.angle_deg(l2)
    assert approx(math.pi / 4) == l1.angle_rad(l2)
    # The angle computation should consider the orientation of the lines:
    assert approx(135) == l1.angle_deg(l2.reversed())

    l2 = pg.Line2d((-1, 0), (-2, 1))
    assert not l1.is_collinear(l2)
    assert not l1.is_parallel(l2)
    assert approx(135) == l1.angle_deg(l2)
    assert approx(45) == l1.angle_deg(l2.reversed())
  
    # Parallel & collinear lines
    l2 = pg.Line2d((-1, 0), (-2, 0))
    assert l1.is_collinear(l2)
    assert l1.is_parallel(l2)
    assert approx(180) == l1.angle_deg(l2)
    assert approx(0) == l1.angle_deg(l2.reversed())
  
    # Parallel / non-collinear
    l2 = pg.Line2d((-1, -1), (-2, -1))
    assert not l1.is_collinear(l2)
    assert l1.is_parallel(l2)
    assert approx(180) == l1.angle_deg(l2)
    assert approx(0) == l1.angle_deg(l2.reversed())
  
    # Tilt the line:
    l2 = l1.tilt_deg(5)
    assert approx(5) == l1.angle_deg(l2)

    l2 = l1.tilt_deg(75)
    assert approx(75) == l1.angle_deg(l2)

    l2 = l1.tilt_deg(135)
    assert approx(135) == l1.angle_deg(l2)

    l2 = l1.tilt_deg(179)
    assert approx(179) == l1.angle_deg(l2)

    l2 = l1.tilt_deg(182)
    assert approx(178) == l1.angle_deg(l2)

    l2 = l1.tilt_deg(330)
    assert approx(30) == l1.angle_deg(l2)


# TODO: add tests for 3D line
