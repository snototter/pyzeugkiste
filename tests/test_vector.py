import pytest
import pyzeugkiste  # Needed for the deserialization checks, i.e. eval(repr(x))
from pyzeugkiste import geo as pzgeo
import math
import pickle
import numpy as np


def vector_test_helper(vec, zero):
    assert eval(repr(vec)) == vec

    assert vec.ndim >= 2
    assert vec.ndim == zero.ndim

    # Test indexing
    with pytest.raises(IndexError):
        vec[vec.ndim]

    with pytest.raises(IndexError):
        vec[-vec.ndim - 1]

    for i in range(vec.ndim):
        assert vec[vec.ndim - i - 1] == vec[-(i+1)]

    # Make a copy for later
    cp = vec.copy()
    assert cp == vec

    # Basic arithmetics
    vec2 = vec + vec
    assert vec != vec2
    assert 2 * vec == vec2

    assert (vec + zero) == vec

    # Multiplying with an integer must yield the same vector type
    other = vec.copy()
    other *= 2
    assert other == vec2
    assert other.dtype == vec.dtype


    # Division will always yield a floating point vector
    other = vec / 2
    assert other.dtype == np.float64
    result = other / 1.2
    assert 2.4 * result == other * 2
    assert result.dtype == np.float64

    other /= 2
    assert other == vec / 4
    assert other.dtype == np.float64


    vec3 = vec + vec + cp
    assert vec3 == 3 * vec
    assert (vec3 - vec) == (2 * vec)

    # Negation
    neg = -vec
    assert vec == cp
    for i in range(vec.ndim):
        assert neg[i] == pytest.approx(-vec[i])

    # Absolute
    abs_vec = abs(vec)
    abs_neg = abs(neg)
    assert abs_vec == abs_neg
    for i in range(vec.ndim):
        assert abs_vec[i] >= 0

    # Distance/Length & dot product:
    dot1 = vec.dot(vec)
    dot3 = vec.dot(vec3)
    assert 3 * dot1 == pytest.approx(dot3)

    assert vec.unit_vector().length() == pytest.approx(1)

    length = vec.length()
    assert math.sqrt(dot1) == pytest.approx(length)
    assert dot1 == pytest.approx(vec.length_squared())

    dist = vec.distance_l2(zero)
    assert dist == pytest.approx(length)

    dist = vec.distance_l1(zero)
    assert dist == pytest.approx(sum(abs(vec)))

    vec2 = 4.0 * vec
    dist = vec.distance_l2(vec2)
    assert 3 * length == pytest.approx(dist)

    diff = vec2 - vec
    assert vec.distance_l1(vec2) == pytest.approx(sum(abs(diff)))

    # Directions
    assert vec2.direction_vector(vec) == -vec.direction_vector(vec2)
    assert vec2.direction_vector(vec) != vec3.direction_vector(vec)
    assert vec2.direction_vector(vec).unit_vector() == vec3.direction_vector(vec).unit_vector()
    assert vec2.direction_vector(vec).unit_vector().length() == pytest.approx(1)
    assert vec3.direction_vector(vec).unit_vector().length() == pytest.approx(1)
    assert vec2.direction_vector(vec).unit_vector() == -vec.direction_vector(vec3).unit_vector()

    # Cross product (only for 3d)
    other = zero.copy()
    for i in range(vec.ndim):
      other[i] = i

    if vec.ndim != 3:
      with pytest.raises(AttributeError):
        vec.cross(other)
    else:
      cross = vec.cross(other)
      expected = (
            vec.y * other.z - vec.z * other.y,
            vec.z * other.x - vec.x * other.z,
            vec.x * other.y - vec.y * other.x)
      assert cross[0] == pytest.approx(expected[0])
      assert cross[1] == pytest.approx(expected[1])
      assert cross[2] == pytest.approx(expected[2])


def check_vector_zero(vec):
    for i in range(vec.ndim):
        assert vec[i] == pytest.approx(0)


def test_vectors():
    zero2d = pzgeo.Vec2d()
    assert zero2d.ndim == 2
    check_vector_zero(zero2d)

    zero3d = pzgeo.Vec3d()
    assert zero3d.ndim == 3
    check_vector_zero(zero3d)

    vec = pzgeo.Vec2d(99, -45)
    assert vec.max_value() == pytest.approx(99)
    assert vec.max_index() == 0
    assert vec.min_value() == pytest.approx(-45)
    assert vec.min_index() == 1
    vector_test_helper(vec, zero2d)

    vec = pzgeo.Vec3d(0.01, 0.99, -15.3)
    assert vec.max_value() == pytest.approx(0.99)
    assert vec.max_index() == 1
    assert vec.min_value() == pytest.approx(-15.3)
    assert vec.min_index() == 2
    vector_test_helper(vec, zero3d)

    vector_test_helper(pzgeo.Vec3d(1, 2, 3), zero3d)
    v3d = pzgeo.Vec3d(-171, 2, -3)
    assert v3d.max_value() == pytest.approx(2)
    assert v3d.max_index() == 1
    assert v3d.min_value() == pytest.approx(-171)
    assert v3d.min_index() == 0
    vector_test_helper(v3d, zero3d)

    v2d = pzgeo.Vec2d(23, -17)
    vector_test_helper(v2d, zero2d)
    u2d = v2d.unit_vector()
    assert u2d.length() == pytest.approx(1)
    assert u2d.x == pytest.approx(23 / 28.600699292)
    assert u2d.y == pytest.approx(-17 / 28.600699292)

    assert v2d.direction_vector(zero2d) == -v2d
    assert v2d.direction_vector(v2d) == zero2d

    orth = v2d.copy()
    orth[0] = v2d[1]
    orth[1] = -v2d[0]
    assert orth != v2d
    assert v2d.dot(orth) == pytest.approx(0)

    # We can cast tuples/lists to Vec
    assert v2d == pzgeo.Vec2d((23, -17))  # tuple
    assert v2d == pzgeo.Vec2d([23, -17])  # list

    assert v3d == pzgeo.Vec3d((-171, 2, -3))  # tuple
    assert v3d == pzgeo.Vec3d([-171, 2, -3])  # list

    # In python, we can even support named arguments
    assert v2d == pzgeo.Vec2d(y=-17, x=23)
    with pytest.raises(TypeError):
        assert v2d == pzgeo.Vec2d(x=3, y=4, z=7)

    # ... and this also works for 3d vectors:
    assert v3d == pzgeo.Vec3d(y=2, x=-171, z=-3)
    with pytest.raises(TypeError):
        pzgeo.Vec3d(x=3, y=4)
    with pytest.raises(TypeError):
        pzgeo.Vec3d(x=3, y=4, z=7, w=99)

    # Division can yield inf/nan
    v = pzgeo.Vec3d(0, -1e10, 3)
    result = 1 / v
    assert math.isinf(result[0])
    assert not math.isinf(result[1])
    assert result[1] == pytest.approx(-1e-10)
    assert result[2] == pytest.approx(1/3)

    result = 0 / v
    assert math.isnan(result[0])
    assert result[1] == pytest.approx(0)
    assert result[2] == pytest.approx(0)


def test_pickling():
    vecs = [pzgeo.Vec2d(99, -45),
            pzgeo.Vec2d(0.0001, 0.0789),
            pzgeo.Vec2d(-0.03, -0.01234),
            pzgeo.Vec3d(1, 2, 3),
            pzgeo.Vec3d(-0.001, -0.0002, -0.0003),
            pzgeo.Vec3d([1, 2, 3]),
            pzgeo.Vec2i([-1234, 42]),
            pzgeo.Vec3i([1234000, -1234, 3567])]

    for vec in vecs:
        data = pickle.dumps(vec)
        restored = pickle.loads(data)
        assert vec == restored

        assert eval(repr(vec)) == vec


def test_vector_conversion():
    from sys import byteorder
    supported_endian = '<' if (byteorder == 'little') else '>'
    not_supported_endian = '>' if (byteorder == 'little') else '<'

    types = [
        'i1', 'i2', 'i4', 'i8', 'l',
        'u1', 'u2', 'u4', 'u8',
        'f4', 'f8', f'{supported_endian}f8', 'd']
    for tp in types:
        print(f'Testing "{tp}"')
        x = np.array([1, 2], dtype=tp)
        vec = pzgeo.Vec2d(x)
        assert vec.x == pytest.approx(1.0)
        assert vec.y == pytest.approx(2.0)
        assert vec.dtype == np.float64

        y = np.array([[3], [4]], dtype=tp)
        vec = pzgeo.Vec2d(y)
        assert vec.x == pytest.approx(3.0)
        assert vec.y == pytest.approx(4.0)

        vec = pzgeo.Vec2i(x)
        assert vec.x == 1
        assert vec.y == 2
        assert vec.dtype == np.int32

        vec = pzgeo.Vec2i(y)
        assert vec.x == 3
        assert vec.y == 4

    # Special type conversion (from bool array)
    vec = pzgeo.Vec2i(np.array([1, 2], dtype=bool))
    assert vec.x == 1
    assert vec.y == 1

    vec = pzgeo.Vec2i(np.array([0, 2], dtype=bool))
    assert vec.x == 0
    assert vec.y == 1

    vec = pzgeo.Vec2i(np.array([9, 0], dtype='?'))
    assert vec.x == 1
    assert vec.y == 0

    # Currently not supported numpy dtypes:
    # 'f2' -- Haven't investigated (and am currently not interested in) half-
    #   precision floating point types in C++
    # '>f8' -- "Wrong" endianness for your system; don't see a reason why I
    #   should support this currently.
    types = [
        f'{not_supported_endian}i4', f'{not_supported_endian}i8',
        'f2', f'{not_supported_endian}f4', f'{not_supported_endian}f8']
    for tp in types:
        x = np.array([1, 2], dtype=tp)
        with pytest.raises(ValueError):
            vec = pzgeo.Vec2d(x)


def test_vector_numpy():
    """Vector construction from numpy arrays & basic math/type interchangability"""
    x = np.array([1, 2, 3])
    y = np.array([4, 5, 6])
    expected_elem = x * y
    expected_dot = x @ y

    vy = pzgeo.Vec3d(y)
    res_a = x * vy
    assert np.array_equal(expected_elem, res_a)

    vx = pzgeo.Vec3d(x)
    res_b = vx * y
    assert np.array_equal(expected_elem, res_b)

    res_a = x @ vy
    assert pytest.approx(expected_dot) == res_a

    res_b = vx @ y
    assert pytest.approx(expected_dot) == res_b

    assert pytest.approx(expected_dot) == vx.dot(vy)

    for dt in [np.uint8, np.int16, np.uint16, np.int32, np.uint32,
               np.int64, np.uint64, np.float32, np.float64]:
        npv = np.array([1, 2], dtype=dt)
        v = pzgeo.Vec2d(npv)
        assert v[0] == float(npv[0])
        assert v[1] == float(npv[1])

        npv = np.array([[1], [2]], dtype=dt)
        v = pzgeo.Vec2d(npv)
        assert v[0] == float(npv[0])
        assert v[1] == float(npv[1])

        npv = np.array([[1, 2, -3], [4, -5, 6]]).astype(dt)

        with pytest.raises(ValueError):
            v = pzgeo.Vec2d(npv[0, :])

        v = pzgeo.Vec2d(npv[:, 0])
        assert v[0] == float(npv[0, 0])
        assert v[1] == float(npv[1, 0])

        v = pzgeo.Vec2d(npv[:, 1])
        assert v[0] == float(npv[0, 1])
        assert v[1] == float(npv[1, 1])

        v = pzgeo.Vec2d(npv[:, 2])
        assert v[0] == float(npv[0, 2])
        assert v[1] == float(npv[1, 2])


def test_vector_numpy_sliced():
    """Construct a vector from slices of a numpy array."""
    x = np.array(
        [[1, 2, 3], [4, 5, 6], [7, 8, 9]], dtype=np.float64)

    for idx in range(3):
        v = pzgeo.Vec3d(x[:, idx])
        data = x[:, idx]
        assert not data.flags.c_contiguous
        assert v == pzgeo.Vec3d(data[0], data[1], data[2])

        v = pzgeo.Vec3d(x[idx, :])
        data = x[idx, :]
        assert data.flags.c_contiguous
        assert v == pzgeo.Vec3d(data[0], data[1], data[2])

#def test_vector_numpy_noncont():
#    """Construct a vector from non-C-contiguous arrays."""
#TODO
