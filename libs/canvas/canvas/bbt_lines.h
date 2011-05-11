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

/** @file  canvas/bbt_lines.cc
 *  @brief Declaration of an item to draw bar/beat lines.
 */

#include "canvas/item.h"
#include "canvas/outline.h"

namespace ARDOUR {
	class Session;
}

namespace ArdourCanvas {

/** Canvas item to draw bar and beat lines.  I think it's easier
 *  to have a specific object rather than using a group with lines
 *  as the item extends to infinite width.
 */
class BBTLines : virtual public Item, public Outline
{
public:
	BBTLines (Group *, Color, Color);

	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	void compute_bbox () const;
	XMLNode* get_state () const;
	void set_state (XMLNode const *);

	void set_session (ARDOUR::Session *);

	void set_frames_per_pixel (double);
	void tempo_map_changed ();

private:
	/** Our session */
	ARDOUR::Session* _session;
	/** Frames per pixel */
	double _frames_per_pixel;
	/** Color to use for bar lines */
	Color _bar_color;
	/** Color to use for beat lines */
	Color _beat_color;
};

}
