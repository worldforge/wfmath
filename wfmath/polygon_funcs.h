// polygon_funcs.h (line polygon implementation)
//
//  The WorldForge Project
//  Copyright (C) 2002  The WorldForge Project
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
//  For information about WorldForge and its authors, please contact
//  the Worldforge Web Site at http://www.worldforge.org.
//

// Author: Ron Steinke

#ifndef WFMATH_POLYGON_FUNCS_H
#define WFMATH_POLYGON_FUNCS_H

#include <wfmath/const.h>
#include <wfmath/vector.h>
#include <wfmath/point.h>
#include <wfmath/axisbox.h>
#include <wfmath/ball.h>
#include <wfmath/polygon.h>

namespace WFMath {

template<const int dim>
_Poly2Orient<dim>& _Poly2Orient<dim>::operator=(const _Poly2Orient<dim>& a)
{
  m_origin_valid = a.m_origin_valid;
  if(m_origin_valid)
    m_origin = a.m_origin;

  for(int i = 0; i < 2; ++i) {
    m_axes_valid[i] = a.m_axes_valid[i];
    if(m_axes_valid[i])
      m_axes[i] = a.m_axes[i];
  }

  return *this;
}

template<const int dim>
bool Polygon<dim>::isEqualTo(const Polygon<dim>& p, double epsilon) const
{
  // The same polygon can be expressed in different ways in the interal
  // format, so we have to call getCorner();

  int size = m_poly.numCorners();
  if(size != p.m_poly.numCorners())
    return false;

  for(int i = 0; i < size; ++i)
    if(!Equal(getCorner(i), p.getCorner(i), epsilon))
      return false;

  return true;
}

// WARNING! This operator is for sorting only. It does not
// reflect any property of the box.
template<const int dim>
bool Polygon<dim>::operator< (const Polygon<dim>& s) const
{
  int size = m_poly.numCorners(), s_size = s.m_poly.numCorners();

  if(size != s_size)
    return size < s_size;

  for(int i = 0; i < size; ++i) {
    Point<dim> p = getCorner(i), sp = s.getCorner(i);
    if(p != sp)
      return p < sp;
  }

  return false;
}

template<const int dim>
Point<dim> _Poly2Orient<dim>::convert(const Point<2>& p) const
{
  Point<dim> out = m_origin;

  for(int j = 0; j < 2; ++j) {
    if(m_axes_valid[j])
      out += m_axes[j] * p[j];
    else
      assert(p[j] == 0);
  }

  return out;
}

template<const int dim>
bool _Poly2Orient<dim>::expand(const Point<dim>& pd, Point<2>& p2, double epsilon)
{
  p2[0] = p2[1] = 0; // initialize

  if(!m_origin_valid) { // Adding to an empty list
    m_origin = pd;
    m_origin_valid = true;
    return true;
  }

  Vector<dim> shift = pd - m_origin, start_shift = shift;

  CoordType bound = shift.sqrMag() * epsilon;

  int j = 0;

  while(true) {
    if(Dot(shift, start_shift) <= bound) // shift is effectively zero
      return true;

    if(j == 2) // Have two axes, shift doesn't lie in their plane
      return false;

    if(!m_axes_valid[j]) { // Didn't span this previously, now we do
      p2[j] = shift.mag();
      m_axes[j] = shift / p2[j];
      m_axes_valid[j] = true;
      return true;
   }

   p2[j] = Dot(shift, m_axes[j]);
   shift -= m_axes[j] * p2[j]; // shift is now perpendicular to m_axes[j]
   ++j;
  }
}

template<const int dim>
_Poly2Reorient _Poly2Orient<dim>::reduce(const Polygon<2>& poly, int skip)
{
  if(poly.numCorners() <= ((skip == 0) ? 1 : 0)) { // No corners left
    m_origin_valid = false;
    m_axes_valid[0] = false;
    m_axes_valid[1] = false;
    return _WFMATH_POLY2REORIENT_ALL;
  }

  assert(m_origin_valid);

  // Check that we still span both axes

  bool still_valid[2] = {false,}, got_ratio = false;
  CoordType ratio;

  int i = 0, end = poly.numCorners();
  if(skip == 0)
    ++i;
  assert(i != end);
  Point<2> first_point = m_poly[i];

  while(++i != end) {
    if(i == skip)
      continue;

    Vector<2> diff = m_poly[i] - first_point;
    if(diff == 0) // No addition to span
      continue;
    for(int j = 0; j < 2; ++j) {
      if(diff[j] == 0) {
        assert(diff[j ? 0 : 1] != 0); // because diff != 0
        if(still_valid[j ? 0 : 1] || got_ratio) // We span a 2D space
          return _WFMATH_POLY2REORIENT_NONE;
        still_valid[j] = true;
      }
      // The point has both elements nonzero
      if(still_valid[0] || still_valid[1]) // We span a 2D space
        return _WFMATH_POLY2REORIENT_NONE;
      CoordType new_ratio = diff[1] / diff[0];
      if(!got_ratio) {
        ratio = new_ratio;
        got_ratio = true;
        continue;
      }
      if(!Equal(ratio, new_ratio)) // We span a 2D space
        return _WFMATH_POLY2REORIENT_NONE;
    }
  }

  // Okay, we don't span both vectors. What now?

  if(still_valid[0]) {
    assert(!still_valid[1]);
    assert(!got_ratio);
    // This is easy, m_axes[1] is just invalid
    m_axes_valid[1] = false;
    if(first_point[1] != 0) { // Need to shift points
      m_origin += m_axes[1] * first_point[1];
      return _WFMATH_POLY2REORIENT_CLEAR_AXIS2;
    }
    else
      return _WFMATH_POLY2REORIENT_NONE;
  }

  if(still_valid[1]) {
    assert(!got_ratio);
    // This is a little harder, m_axes[0] is invalid, must swap axes
    if(first_point[0] != 0)
      m_origin += m_axes[0] * first_point[0];
    m_axes[0] = m_axes[1];
    m_axes_valid[1] = false;
    return _WFMATH_POLY2REORIENT_MOVE_AXIS2_TO_AXIS1;
  }

  if(!got_ratio) { // Nothing's valid, all points are equal
    bool shift_points[2] = {false,};
    for(int j = 0; j < 2; ++j) {
      if(first_point[j] != 0)
        shift_points = true;
      m_axes_valid[j] = false;
    }
    if(shift_points[0])
      return shift_points[1] ? _WFMATH_POLY2REORIENT_CLEAR_BOTH_AXES
			     : _WFMATH_POLY2REORIENT_CLEAR_AXIS1;
    else
      return shift_points[1] ? _WFMATH_POLY2REORIENT_CLEAR_AXIS2
			     : _WFMATH_POLY2REORIENT_NONE;
  }

  // All the points are colinear, along some line which is not parallel
  // to either of the original axes

  m_axes[0] += m_axes[1] * ratio;
  m_axes_valid[1] = false;
  return _Poly2Reorient(_WFMATH_POLY2REORIENT_SCALE1_CLEAR2, sqrt(1 + ratio * ratio));
}

template<const int dim>
void _Poly2Orient<dim>::rotate(const RotMatrix<dim>& m, const Point<dim>& p)
{
  if(m_origin_valid)
    m_origin.rotate(m, p);

  for(int j = 0; j < 2; ++j)
    if(m_axes_valid[j])
      m_axes[j] = Prod(m_axes[j], m);
}

template<const int dim>
void _Poly2Orient<dim>::rotate(const RotMatrix<dim>& m, const Point<2>& p)
{
  assert(m_origin_valid);

  if(!m_axes_valid[0]) {
    assert(p[0] == 0 && p[1] == 0);
    return;
  }

  Vector<dim> shift = m_axes[0] * p[0];
  m_axes[0] = Prod(m_axes[0], m);

  if(m_axes_valid[1]) {
    shift += m_axes[1] * p[1];
    m_axes[1] = Prod(m_axes[1], m);
  }
  else
    assert(p[1] == 0);

  m_origin += shift - Prod(shift, m);
}

template<const int dim>
Vector<dim> _Poly2Orient<dim>::offset(const Point<dim>& pd, Point<2>& p2) const
{
  assert(m_origin_valid); // Check for empty polygon before calling this

  Vector<dim> out = pd - m_origin;

  for(int j = 0; j < 2; ++j) {
    p2[j] = Dot(out, m_axes[j]);
    out -= p2[j] * m_axes[j];
  }

  return out;
}

template<>
bool _Poly2Orient<3>::checkIntersectPlane(const AxisBox<3>& b, Point<2>& p2) const;

template<const int dim>
bool _Poly2Orient<dim>::checkIntersect(const AxisBox<dim>& b, Point<2>& p2,
				       bool proper) const
{
  assert(m_origin_valid);

  if(!m_axes_valid[0]) {
    // Single point
    p2[0] = p2[1] = 0;
    return Intersect(b, convert(p2), proper);
  }

  if(m_axes_valid[1]) {
    // A plane

    // I only know how to do this in 3D, so write a function which will
    // specialize to different dimensions

    return checkIntersectPlane(b, p2) && (!proper || Contains(b, p2, true));
  }

  // A line

  // This is a modified version of AxisBox<>/Segment<> intersection

  CoordType min = 0, max = 0; // Initialize to avoid compiler warnings
  bool got_bounds = false;

  for(int i = 0; i < dim; ++i) {
    const CoordType dist = (m_axes[0])[i]; // const may optimize away better
    if(dist == 0) {
      if(_Less(m_origin[i], b.lowCorner()[i], proper)
        || _Greater(m_origin[i], b.highCorner()[i], proper))
        return false;
    }
    else {
      CoordType low = (b.lowCorner()[i] - m_origin[i]) / dist;
      CoordType high = (b.highCorner()[i] - m_origin[i]) / dist;
      if(low > high) {
        CoordType tmp = high;
        high = low;
        low = tmp;
      }
      if(got_bounds) {
        if(low > min)
          min = low;
        if(high < max)
          max = high;
      }
      else {
        min = low;
        max = high;
        got_bounds = true;
      }
    }
  }

  assert(got_bounds); // We can't be parallel in _all_ dimensions

  if(_LessEq(min, max, proper)) {
    p2[0] = (max - min) / 2;
    p2[1] = 0;
    return true;
  }
  else
    return false;
}

template<const int dim>
bool Polygon<dim>::addCorner(int i, const Point<dim>& p, double epsilon)
{
  Point<2> p2;
  bool succ = m_orient.expand(p, p2, epsilon);
  if(succ)
    m_poly.addCorner(i, p2, epsilon);
  return succ;
}

template<const int dim>
void Polygon<dim>::removeCorner(int i)
{
  m_poly.removeCorner(i);
  _Poly2Reorient r = m_orient.reduce(m_poly);
  r.reorient(m_poly);
}

template<const int dim>
bool Polygon<dim>::moveCorner(int i, const Point<dim>& p, double epsilon)
{
  _Poly2Orient<dim> try_orient = m_orient;
  _Poly2Reorient r = try_orient.reduce(m_poly, i);
  Point<2> p2;

  if(!try_orient.expand(p, p2, epsilon))
    return false;

  r.reorient(m_poly, i);
  m_poly[i] = p2;
  m_orient = try_orient;

  return true;
}

template<const int dim>
AxisBox<dim> Polygon<dim>::boundingBox() const
{
  assert(m_poly.numCorners() > 0);

  Point<dim> min = m_orient.convert(m_poly[0]), max = min;

  for(int i = 1; i != m_poly.numCorners(); ++i) {
    Point<dim> p = m_orient.convert(m_poly[i]);
    for(int j = 0; j < dim; ++j) {
      if(p[j] < min[j])
        min[j] = p[j];
      if(p[j] > max[j])
        max[j] = p[j];
    }
  }

  return AxisBox<dim>(min, max, true);
}

template<const int dim>
Ball<dim> Polygon<dim>::boundingSphere() const
{
  Ball<2> b = m_poly.boundingSphere();

  return Ball<dim>(m_orient.convert(b.center()), b.radius());
}

template<const int dim>
Ball<dim> Polygon<dim>::boundingSphereSloppy() const
{
  Ball<2> b = m_poly.boundingSphereSloppy();

  return Ball<dim>(m_orient.convert(b.center()), b.radius());
}

} // namespace WFMath

#endif  // WFMATH_POLYGON_FUNCS_H
