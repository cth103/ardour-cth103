/*
    Copyright (C) 2011 Paul Davis
    Author: Carl Hetherington <cth@carlh.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/** @file  canvas/text.cc
 *  @brief Implementation of various basic types.
 */

#include <algorithm>
#include <cfloat>
#include <cassert>
#include "canvas/types.h"

using namespace std;
using namespace Canvas;

Coord const Canvas::COORD_MAX = DBL_MAX;
TransformIndex const Canvas::IDENTITY = -1;

/** A fairly dumb but effective add operator which avoids
 *  overflow in some circumstances (namely if a or b are COORD_MAX).
 *  @param a First operand.
 *  @param b Second operand.
 *  @return a + b (unless a == COORD_MAX or b == COORD_MAX, in which case the result is COORD_MAX).
 */
Coord
Canvas::safe_add (Coord a, Coord b)
{
	if (a == COORD_MAX || b == COORD_MAX) {
		return COORD_MAX;
	}

	return a + b;
}

/** Translate a Duple.
 *  @param t Translation.
 *  @return Translated version of *this.
 */
Duple
Duple::translate (Duple t) const
{
	Duple d;

	d.x = safe_add (x, t.x);
	d.y = safe_add (y, t.y);
	
	return d;
}

/** Scale a Duple.
 *  @param s Scale.
 *  @return Scaled version of *this.
 */
Duple
Duple::scale (Duple s) const
{
	Duple d;

	/* XXX: may overflow */
	d.x = d.x * s.x;
	d.y = d.y * s.y;

	return d;
}

/** operator== for Duples */
bool
Duple::operator== (Duple const & d) const
{
	return (x == d.x && y == d.y);
}

/** Compute the intersection of *this with another Rect.
 *  @param o Other Rect.
 *  @return Intersection of *this and o.
 */
boost::optional<Rect>
Rect::intersection (Rect const & o) const
{
	Rect i;
	
	i.x0 = max (x0, o.x0);
	i.y0 = max (y0, o.y0);
	i.x1 = min (x1, o.x1);
	i.y1 = min (y1, o.y1);

	if (i.x0 > i.x1 || i.y0 > i.y1) {
		return boost::optional<Rect> ();
	}
	
	return boost::optional<Rect> (i);
}

/** Translate a Rect.
 *  @param t Translation.
 *  @return Translated version of *this.
 */
Rect
Rect::translate (Duple t) const
{
	Rect r;
	
	r.x0 = safe_add (x0, t.x);
	r.y0 = safe_add (y0, t.y);
	r.x1 = safe_add (x1, t.x);
	r.y1 = safe_add (y1, t.y);
	return r;
}

/** Scale a Rect.
 *  @param s Scale.
 *  @return Scaled version of *this.
 */
Rect
Rect::scale (Duple s) const
{
	Rect r;

	/* XXX: may overflow */
	r.x0 = x0 * s.x;
	r.y0 = y0 * s.y;
	r.x1 = x1 * s.x;
	r.y1 = y1 * s.y;
	return r;
}

/** Extend *this so that it covers another Rect.
 *  @param o Rect to expand to cover.
 *  @return Extended Rect.
 */
Rect
Rect::extend (Rect const & o) const
{
	Rect r;
	r.x0 = min (x0, o.x0);
	r.y0 = min (y0, o.y0);
	r.x1 = max (x1, o.x1);
	r.y1 = max (y1, o.y1);
	return r;
}

/** Extend *this by a given amount in all directions.
 *  @param amount Amount to extend by.
 *  @return Extended Rect.
 */
Rect
Rect::expand (Distance amount) const
{
	Rect r;
	r.x0 = x0 - amount;
	r.y0 = y0 - amount;
	r.x1 = safe_add (x1, amount);
	r.y1 = safe_add (y1, amount);
	return r;
}

/** @param point Test point.
 *  @return true if *this contains point, otherwise false.
 */
bool
Rect::contains (Duple point) const
{
	return point.x >= x0 && point.x <= x1 && point.y >= y0 && point.y <= y1;
}

/** `Fix' a Rect so that x0 <= x1 and y0 <= y1.
 *  @return `Fixed' Rect.
 */
Rect
Rect::fix () const
{
	Rect r;
	
	r.x0 = min (x0, x1);
	r.y0 = min (y0, y1);
	r.x1 = max (x0, x1);
	r.y1 = max (y0, y1);

	return r;
}

/** operator== for Rects */
bool
Rect::operator== (Rect const & other) const
{
	return (x0 == other.x0 && x1 == other.x1 && y0 == other.y0 && y1 == other.y1);
}

/** operator!= for Rects */
bool
Rect::operator!= (Rect const & other) const
{
	return (x0 != other.x0 || x1 != other.x1 || y0 != other.y0 || y1 != other.y1);
}

/** Unary operator- for Duples */
Duple
Canvas::operator- (Duple const & o)
{
	return Duple (-o.x, -o.y);
}

/** Binary operator+ for Duples */
Duple
Canvas::operator+ (Duple const & a, Duple const & b)
{
	return Duple (safe_add (a.x, b.x), safe_add (a.y, b.y));
}

/** Binary operator- for Duples */
Duple
Canvas::operator- (Duple const & a, Duple const & b)
{
	return Duple (a.x - b.x, a.y - b.y);
}

/** Binary operator/ for Duples with a double */
Duple
Canvas::operator/ (Duple const & a, double b)
{
	return Duple (a.x / b, a.y / b);
}

/** Binary operator/ for Duples */
Duple
Canvas::operator/ (Duple const & a, Duple const & b)
{
	return Duple (a.x / b.x, a.y / b.y);
}

/** operator<< for ostreams and Duples */
ostream &
Canvas::operator<< (ostream & s, Duple const & r)
{
	s << "(" << r.x << ", " << r.y << ")";
	return s;
}

/** operator<< for ostreams and Rects */
ostream &
Canvas::operator<< (ostream & s, Rect const & r)
{
	s << "[(" << r.x0 << ", " << r.y0 << "), (" << r.x1 << ", " << r.y1 << ")]";
	return s;
}

