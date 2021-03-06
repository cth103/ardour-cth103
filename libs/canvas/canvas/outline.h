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

/** @file  canvas/outline.cc
 *  @brief Declaration of a mixin for items which have an outline.
 */

#ifndef __CANVAS_OUTLINE_H__
#define __CANVAS_OUTLINE_H__

#include <stdint.h>
#include "canvas/types.h"
#include "canvas/item.h"

namespace Canvas {

/** A mixin class which can be inherited by items that have
 *  an outline (e.g. lines, polygons etc.)
 */
class Outline : virtual public Item
{
public:
	Outline (Group *);
	Outline (Group *, TransformIndex);
	virtual ~Outline () {}

	void add_outline_state (XMLNode *) const;
	void set_outline_state (XMLNode const *);
	
	Color outline_color () const {
		return _outline_color;
	}

	virtual void set_outline_color (Color);

	Distance outline_width () const {
		return _outline_width;
	}
	
	virtual void set_outline_width (Distance);

	bool outline () const {
		return _outline;
	}

	void set_outline (bool);

#ifdef CANVAS_COMPATIBILITY
	int& property_first_arrowhead () {
		return _foo_int;
	}
	int& property_last_arrowhead () {
		return _foo_int;
	}
	int& property_arrow_shape_a () {
		return _foo_int;
	}
	int& property_arrow_shape_b () {
		return _foo_int;
	}
	int& property_arrow_shape_c () {
		return _foo_int;
	}
	bool& property_draw () {
		return _foo_bool;
	}
#endif	

protected:

	void setup_outline_context (Cairo::RefPtr<Cairo::Context>) const;

	/** The outline color to use */
	Color _outline_color;
	/** Outline width in pixels */
	Distance _outline_width;
	/** true to plot outline, otherwise false */
	bool _outline;

#ifdef CANVAS_COMPATIBILITY
	int _foo_int;
	bool _foo_bool;
#endif
};

}

#endif
