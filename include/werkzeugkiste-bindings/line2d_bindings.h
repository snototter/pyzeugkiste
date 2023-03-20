#ifndef WERKZEUGKISTE_BINDINGS_GEOMETRY_LINE2D_H
#define WERKZEUGKISTE_BINDINGS_GEOMETRY_LINE2D_H

#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <werkzeugkiste/geometry/primitives.h>
#include <werkzeugkiste/logging.h>

#include <sstream>
#include <stdexcept>
#include <string_view>

namespace werkzeugkiste::bindings {

//-------------------------------------------------
// Line2d

inline pybind11::tuple Line2dToTuple(
    const werkzeugkiste::geometry::Line2d &obj) {
  return pybind11::make_tuple(obj.From(), obj.To());
}

inline werkzeugkiste::geometry::Line2d Line2dFromTupleOrList(
    const pybind11::object &object, std::string_view module_name) {
  if (!pybind11::isinstance<pybind11::tuple>(object) &&
      !pybind11::isinstance<pybind11::list>(object)) {
    std::string s("Cannot cast ");
    s += pybind11::cast<std::string>(object.attr("__class__").attr("__name__"));
    s += " to `";
    s += module_name;
    s += ".Line2d`, expected either tuple or list.";
    throw std::invalid_argument(s);
  }

  const pybind11::tuple tpl = object.cast<pybind11::tuple>();
  if (tpl.size() != 2) {
    std::ostringstream s;
    s << "Cannot create a `" << module_name << ".Line2d` from "
      << pybind11::cast<std::string>(object.attr("__class__").attr("__name__"))
      << " with " << tpl.size()
      << " entries! Only `(point1, point2)` is supported.";
    throw std::invalid_argument(s.str());
  }

  return werkzeugkiste::geometry::Line2d(
      tpl[0].cast<werkzeugkiste::geometry::Vec2d>(),
      tpl[1].cast<werkzeugkiste::geometry::Vec2d>());
}

inline werkzeugkiste::geometry::Line2d Line2dFromTuple(
    const pybind11::tuple &tpl, std::string_view module_name) {
  return Line2dFromTupleOrList(tpl, module_name);
}

inline werkzeugkiste::geometry::Line2d Line2dFromList(
    const pybind11::list &lst, std::string_view module_name) {
  return Line2dFromTupleOrList(lst, module_name);
}

inline void RegisterLine2d(pybind11::module &m) {
  using L = werkzeugkiste::geometry::Line2d;
  const std::string module_name = m.attr("__name__").cast<std::string>();

  std::ostringstream doc;
  doc << "A line (or line segment) in 2D space.\n\n"
         "A line/segment is represented by two :class:`~"
      << module_name << ".Vec2d`.\n\n"
      << "**Corresponding C++ API:** ``" << module_name << "::Line2d``.";
  pybind11::class_<L> line(m, "Line2d", doc.str().c_str());

  std::ostringstream().swap(doc);
  // This docstring cannot(!) be written as raw string.
  // Otherwise, it messes up the sphinx parser.
  doc << "Creates a line from 2 points.\n\nArgs:\n"
         "  pt1: Start point as :class:`~"
      << module_name << ".Vec2d`.\n  pt2: End point as :class:`~" << module_name
      << ".Vec2d`.";
  line.def(pybind11::init<typename L::vec_type, typename L::vec_type>(),
           doc.str().c_str(), pybind11::arg("pt1"), pybind11::arg("pt2"));

  std::ostringstream().swap(doc);
  doc << "Creates a line from a :class:`tuple`.\n\n"
         "This overloaded constructor is required to allow\n"
         "implicit casting from a tuple holding ``(pt1, pt2)``,"
         "where ``pt1`` & ``pt2`` are the start and end points"
         "as :class:`~"
      << module_name << ".Vec2d`.\n";
  line.def(pybind11::init<>([module_name](const pybind11::tuple &tpl) {
             return Line2dFromTuple(tpl, module_name);
           }),
           doc.str().c_str(), pybind11::arg("tpl"));

  std::ostringstream().swap(doc);
  doc << "Creates a line from a :class:`list`.\n\n"
         "This overloaded constructor is required to allow\n"
         "implicit casting from a list holding ``[pt1, pt2]``,"
         "where ``pt1`` & ``pt2`` are the start and end points"
         "as :class:`~"
      << module_name << ".Vec2d`.";
  line.def(pybind11::init<>([module_name](const pybind11::list &lst) {
             return Line2dFromList(lst, module_name);
           }),
           doc.str().c_str(), pybind11::arg("lst"));

  line.def("__str__",
           [](const L &l) {
             std::ostringstream s;
             s << l;
             return s.str();
           })
      .def("__repr__", [](const L &l) {
        std::ostringstream s;
        s << '<' << l << '>';
        return s.str();
      });

  std::ostringstream().swap(doc);
  doc << ":class:`~" << module_name << ".Line2d` instances can be pickled.";
  line.def(pybind11::pickle(&Line2dToTuple,
                            [module_name](const pybind11::tuple &tpl) {
                              return Line2dFromTuple(tpl, module_name);
                            }),
           doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << "Returns ``True`` if the line is valid, *i.e.* "
         ":math:`\\text{length} > 0`.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::IsValid``.";
  line.def("is_valid", &L::IsValid, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << "Returns the interpolated point on the line/segment.\n\n"
         "Computes :math:`\\text{pt}_1 + "
         "\\text{offset} * (\\text{pt}_2 - \\text{pt}_1)`.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::PointAtOffset``.";
  line.def("point_at_offset", &L::PointAtOffset, doc.str().c_str(),
           pybind11::arg("offset"));

  std::ostringstream().swap(doc);
  doc << "Returns a line/segment with flipped start/end points.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::Reversed``.";
  line.def("reversed", &L::Reversed, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << "Returns a line/segment where :attr:`~" << module_name
      << ".Line2d.pt1` is left of\n:attr:`~" << module_name
      << ".Line2d.pt2`.\n\n"
         "If this line is vertical, the points will be sorted such that\n"
         ":attr:`~"
      << module_name
      << ".Line2d.pt1` is above "
         ":attr:`~"
      << module_name
      << ".Line2d.pt2`.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::LeftToRight``.";
  line.def("left_to_right", &L::LeftToRight, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << "Returns the **non-normalized** direction vector from the start "
         "point\n"
         "to the end point.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::Direction``.";
  line.def("direction", &L::Direction, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << "Returns the unit direction vector from the start point to the end "
         "point.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::UnitDirection``.";
  line.def("unit_direction", &L::UnitDirection, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << ":class:`~" << module_name
      << ".Vec2d`: Starting point.\n\n"
         "  **Corresponding C++ API:** ``"
      << module_name << "::Line2d::From`` and ``" << module_name
      << "::Line2d::SetFrom``.";
  line.def_property("pt1", &L::From, &L::SetFrom, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << ":class:`~" << module_name
      << ".Vec2d`: End point.\n\n"
         "  **Corresponding C++ API:** ``"
      << module_name << "::Line2d::To`` and ``" << module_name
      << "::Line2d::SetTo``.";
  line.def_property("pt2", &L::To, &L::SetTo, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << ":class:`~" << module_name << ".Vec2d`: Mid point between :attr:`~"
      << module_name
      << ".Line2d.pt1`\n"
         "  and :attr:`~"
      << module_name
      << ".Line2d.pt2`.\n\n"
         "  This property simply provides convenience access to the mid "
         "point.\n"
         "  Alternatively, use :meth:`~"
      << module_name
      << ".Line2d.point_at_offset` for an\n"
         "  adjustable offset along the line/segment.\n\n"
         "  **Corresponding C++ API:** ``"
      << module_name << "::Line2d::MidPoint``.";
  line.def_property_readonly("mid_point", &L::MidPoint, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << "float: Length of the segment between :attr:`~" << module_name
      << ".Line2d.pt1`\n"
         "  and :attr:`~viren2d.Line2d.pt2`.\n\n"
         "  **Corresponding C++ API:** ``"
      << module_name << "::Line2d::Length``.";
  line.def_property_readonly("length", &L::Length, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << R"docstr(
        Returns the representation of this line in :math:`\mathbb{P}^2`.

        For more details on lines in projective space, refer to
        `Bob Fisher's CVonline <http://homepages.inf.ed.ac.uk/rbf/CVonline/LOCAL_COPIES/BEARDSLEY/node2.html>`__,
        or
        `Stan Birchfield's lecture notes <http://robotics.stanford.edu/~birch/projective/node4.html>`__.
        )docstr"
      << "\n\n**Corresponding C++ API:** ``" << module_name
      << "::Line2d::HomogeneousForm``.\n\n"
      << R"docstr(
        Returns:
          The :class:`~viren2d.Vec3d` as the result of
          :math:`\text{pt1} \times \text{pt2}`.
        )docstr";
  line.def("homogeneous", &L::HomogeneousForm, doc.str().c_str());

  std::ostringstream().swap(doc);
  doc << "Returns the angle :math:`\\alpha \\in [0, \\pi]` between this line "
         "and the\n"
         "given directional vector.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::AngleRad``.";
  line.def("angle_rad", &L::AngleRad, doc.str().c_str(), pybind11::arg("vec"));

  std::ostringstream().swap(doc);
  doc << "Returns the angle :math:`\\alpha \\in [0, 180]` between this line "
         "and the\n"
         "given directional vector.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::AngleDeg``.";
  line.def("angle_deg", &L::AngleDeg, doc.str().c_str(), pybind11::arg("vec"));

  std::ostringstream().swap(doc);
  doc << "Returns ``True`` if the two lines are collinear.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::IsCollinear``.";
  line.def("is_collinear", &L::IsCollinear, doc.str().c_str(),
           pybind11::arg("line"));

  std::ostringstream().swap(doc);
  doc << "Returns the closest point on the **line**, i.e. the projection of "
         "the\n"
         "given point onto this **line**.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::ClosestPointOnLine``.";
  line.def("closest_point_on_line", &L::ClosestPointOnLine, doc.str().c_str(),
           pybind11::arg("point"));

  std::ostringstream().swap(doc);
  doc << "Returns the minimum distance between the point and this **line**.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::DistancePointToLine``.";
  line.def("distance_point_line", &L::DistancePointToLine, doc.str().c_str(),
           pybind11::arg("point"));

  std::ostringstream().swap(doc);
  doc << "Returns the closest point on this **segment**.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::ClosestPointOnSegment``.";
  line.def("closest_point_on_segment", &L::ClosestPointOnSegment,
           doc.str().c_str(), pybind11::arg("point"));

  std::ostringstream().swap(doc);
  doc << "Returns the minimum distance between the point and this "
         "**segment**.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name << "::Line2d::DistancePointToSegment``.";
  line.def("distance_point_segment", &L::DistancePointToSegment,
           doc.str().c_str(), pybind11::arg("point"));

  std::ostringstream().swap(doc);
  doc << "Clips this **line** against the given **axis-aligned rectangle**.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name
      << "::Line2d::ClipLineByRectangle``.\n\n"
         "Args:\n"
         "  top_left: Position of the rectangle's *top-left* corner as\n"
         "    :class:`~"
      << module_name
      << ".Vec2d`.\n"
         "  size: Dimension of the rectangle as :class:`~"
      << module_name
      << ".Vec2d`.\n\n"
         "Returns:\n"
         "  The clipped :class:`~"
      << module_name
      << ".Line2d`. If the line did not intersect\n"
         "  the rectangle, the result will be invalid. Check this\n"
         "  via :meth:`~"
      << module_name << ".Line2d.is_valid`.";
  line.def("clip_line_by_rectangle", &L::ClipLineByRectangle, doc.str().c_str(),
           pybind11::arg("top_left"), pybind11::arg("size"));

  std::ostringstream().swap(doc);
  doc << "Clips this **segment** against the given **axis-aligned "
         "rectangle**.\n\n"
         "**Corresponding C++ API:** ``"
      << module_name
      << "::Line2d::ClipLineSegmentByRectangle``.\n\n"
         "Args:\n"
         "  top_left: Position of the rectangle's *top-left* corner as\n"
         "    :class:`~"
      << module_name
      << ".Vec2d`.\n"
         "  size: Dimension of the rectangle as :class:`~"
      << module_name
      << ".Vec2d`.\n\n"
         "Returns:\n"
         "  The clipped :class:`~"
      << module_name
      << ".Line2d`.\n"
         "  If the segment did not intersect the rectangle, the result\n"
         "  will be invalid. Check this via :meth:`~"
      << module_name << ".Line2d.is_valid`.";
  line.def("clip_segment_by_rectangle", &L::ClipLineSegmentByRectangle,
           doc.str().c_str(), pybind11::arg("top_left"), pybind11::arg("size"));

  std::ostringstream().swap(doc);
  doc << "Tilts the segment around its start point.";  // TODO doc
  line.def("tilt_deg", &L::TiltDeg, doc.str().c_str(), pybind11::arg("angle"));
  // TODO rad version

  // A 2d line can be initialized from a given tuple/list.
  pybind11::implicitly_convertible<pybind11::tuple, L>();
  pybind11::implicitly_convertible<pybind11::list, L>();
}

/*
TODO bindings missing for:

  /// Returns true if the point is left of this line as specified by
  /// pt_from_ --> pt_to_. If you need to distinguish left-of vs. exactly on
  /// the line, pass a valid pointer `is_on_line`.
  bool IsPointLeftOfLine(
      const vec_type &point, bool *is_on_line = nullptr) const;


  /// Returns true if this line intersects the other line and optionally sets
  /// the `intersection_point`.
  bool IntersectionLineLine(
      const Line2d_ &other, vec_type *intersection_point = nullptr) const;


  /// Returns true if this line(!) intersects the other line segment(!) and
  /// optionally sets the `intersection_point`.
  bool IntersectionLineLineSegment(
      const Line2d_ &segment, vec_type *intersection_point = nullptr) const;


  /// Returns true if this line segment(!) intersects the other line segment(!)
  /// and optionally sets the `intersection_point`.
  bool IntersectionLineSegmentLineSegment(
      const Line2d_ &segment, vec_type *intersection_point = nullptr) const;


  /// Returns the number of intersections (0, 1, or 2) of this line and
  /// the circle. Optionally sets the intersection points.
  int IntersectionLineCircle(
      const Circle_<T> &circle,
      vec_type *intersection1 = nullptr, vec_type *intersection2 = nullptr)
const;


  /// Returns the number of intersections (0, 1, or 2) of this line segment(!)
  /// and the circle. Optionally sets the intersection points.
  int IntersectionLineSegmentCircle(
      const Circle_<T> &circle,
      vec_type *intersection1 = nullptr, vec_type *intersection2 = nullptr)
const;

  */

}  // namespace werkzeugkiste::bindings

#endif  // WERKZEUGKISTE_BINDINGS_GEOMETRY_LINE2D_H
