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
 *  @brief Implementation of a text item.
 */

#include <gtkmm/label.h>
#include "pbd/xml++.h"
#include "canvas/text.h"
#include "canvas/canvas.h"
#include "canvas/utils.h"

using namespace std;
using namespace Canvas;

/** Construct a Text item.
 *  @param parent Parent group.
 */
Text::Text (Group* parent)
	: Item (parent)
	, _font_description (0)
	, _color (0x000000ff)
	, _alignment (Pango::ALIGN_LEFT)
{

}

/** Set our text.
 *  @param text New text.
 */
void
Text::set (string const & text)
{
	begin_change ();
	
	_text = text;
	
	_bbox_dirty = true;
	end_change ();
}

void
Text::compute_bbox () const
{
	if (!_canvas || !_canvas->context ()) {
		_bbox = boost::optional<Rect> ();
		_bbox_dirty = false;
		return;
	}
	
	Pango::Rectangle const r = layout (_canvas->context())->get_ink_extents ();
	
	_bbox = Rect (
		r.get_x() / Pango::SCALE,
		r.get_y() / Pango::SCALE,
		(r.get_x() + r.get_width()) / Pango::SCALE,
		(r.get_y() + r.get_height()) / Pango::SCALE
		);
	
	_bbox_dirty = false;
}

/** Create a Pango layout according to our settings.
 *  @param context Cairo context to plot to.
 *  @return Pango layout.
 */
Glib::RefPtr<Pango::Layout>
Text::layout (Cairo::RefPtr<Cairo::Context> context) const
{
	Glib::RefPtr<Pango::Layout> layout = Pango::Layout::create (context);
	layout->set_text (_text);
	layout->set_font_description (_font_description);
	layout->set_alignment (_alignment);
	return layout;
}

void
Text::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	set_source_rgba (context, _color);
	layout (context)->show_in_cairo_context (context);
}

XMLNode *
Text::get_state () const
{
	XMLNode* node = new XMLNode ("Text");
#ifdef CANVAS_DEBUG
	if (!name.empty ()) {
		node->add_property ("name", name);
	}
#endif
	return node;
}

void
Text::set_state (XMLNode const * node)
{
	/* XXX */
}

/** Set the alignment of our text.
 *  @param alignment New alignment.
 */
void
Text::set_alignment (Pango::Alignment alignment)
{
	begin_change ();
	
	_alignment = alignment;

	_bbox_dirty = true;
	end_change ();
}

/** Set the font description of our text.
 *  @param font_description New font description.
 */
void
Text::set_font_description (Pango::FontDescription font_description)
{
	begin_change ();
	
	_font_description = font_description;

	_bbox_dirty = true;
	end_change ();
}

/** Set the color of our text.
 *  @param color New color.
 */
void
Text::set_color (Color color)
{
	begin_change ();

	_color = color;

	end_change ();
}

		
