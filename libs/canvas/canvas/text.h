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
 *  @brief Declaration of a text item.
 */

#include <pangomm/fontdescription.h>
#include <pangomm/layout.h>
#include "canvas/item.h"

namespace Canvas {

/** An Item which displays some text */
class Text : public Item
{
public:
	Text (Group *);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	void compute_bbox () const;
	XMLNode* get_state () const;
	void set_state (XMLNode const *);

	void set (std::string const &);
	void set_color (uint32_t);
	void set_font_description (Pango::FontDescription);
	void set_alignment (Pango::Alignment);

private:
	Glib::RefPtr<Pango::Layout> layout (Cairo::RefPtr<Cairo::Context>) const;

	/** Our text */
	std::string _text;
	/** Our font */
	Pango::FontDescription _font_description;
	/** Our color */
	uint32_t _color;
	/** Our alignment */
	Pango::Alignment _alignment;
};

}
