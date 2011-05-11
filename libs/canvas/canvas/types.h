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
 *  @brief Declaration of various basic types.
 */

#ifndef __CANVAS_TYPES_H__
#define __CANVAS_TYPES_H__

#include <iostream>
#include <vector>
#include <stdint.h>
#include <boost/optional.hpp>

namespace ArdourCanvas
{

/** A coordinate */
typedef double Coord;
/** A distance */
typedef double Distance;
/** A color */	
typedef uint32_t Color;
/** The maximum value that a Coord can have */
extern Coord const COORD_MAX;
/** The index of a transform in a Group */	
typedef int TransformIndex;
/** The index of the identity transform */
extern TransformIndex const IDENTITY;

extern Coord safe_add (Coord, Coord);

/** A pair of Coords which can be used for a coordinate
 *  or a width/height pair, for example.
 */
struct Duple
{
	Duple ()
		: x (0)
		, y (0)
	{}
	
	Duple (Coord x_, Coord y_)
		: x (x_)
		, y (y_)
	{}
		     
	Coord x;
	Coord y;

	Duple translate (Duple) const;
	Duple scale (Duple) const;

	bool operator== (Duple const &) const;
};


extern Duple operator- (Duple const &);
extern Duple operator+ (Duple const &, Duple const &);
extern Duple operator- (Duple const &, Duple const &);
extern Duple operator/ (Duple const &, double);
extern Duple operator/ (Duple const &, Duple const &);
extern std::ostream & operator<< (std::ostream &, Duple const &);

/** A rectangle of Coords */
struct Rect
{
	Rect ()
		: x0 (0)
		, y0 (0)
		, x1 (0)
		, y1 (0)
	{}
	
	Rect (Coord x0_, Coord y0_, Coord x1_, Coord y1_)
		: x0 (x0_)
		, y0 (y0_)
		, x1 (x1_)
		, y1 (y1_)
	{}

	/** x0 (may be greater than x1) */
	Coord x0;
	/** y0 (may be greater than y1) */
	Coord y0;
	/** x1 (may be less than x0) */
	Coord x1;
	/** y1 (may be less than y0) */
	Coord y1;

	boost::optional<Rect> intersection (Rect const &) const;
	Rect extend (Rect const &) const;
	Rect translate (Duple) const;
	Rect scale (Duple) const;
	Rect expand (Distance) const;
	bool contains (Duple) const;
	Rect fix () const;

	Distance width () const {
		return x1 - x0;
	}

	Distance height () const {
		return y1 - y0;
	}

	bool operator== (Rect const &) const;
	bool operator!= (Rect const &) const;
};

extern std::ostream & operator<< (std::ostream &, Rect const &);

/** A vector of Duples */
typedef std::vector<Duple> Points;

/** An affine transform consisting of x/y scaling factors and a translation */
struct Transform
{
	Transform (Duple scale_, Duple translation_) : scale (scale_), translation (translation_) {}

	/** Scale */
	Duple scale;
	/** Translation */
	Duple translation;
};

}
	
#endif
