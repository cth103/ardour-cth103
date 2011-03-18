/*
    Copyright (C) 2000 Paul Davis

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

#include <cstdlib>
#include <cmath>

#include "utils.h"
#include "editor_cursors.h"
#include "editor.h"

using namespace ARDOUR;
using namespace PBD;
using namespace Gtk;

EditorCursor::EditorCursor (Editor& ed, bool (Editor::*callbck)(GdkEvent*,ArdourCanvas::Item*))
	: editor (ed),
	  canvas_item (editor.cursor_group),
	  length(1.0)
{
	canvas_item.set (
		ArdourCanvas::Point (-1.0, 0.0),
		ArdourCanvas::Point (1.0, 1.0)
		);

	canvas_item.property_width_pixels() = 1;
	canvas_item.property_first_arrowhead() = TRUE;
	canvas_item.property_last_arrowhead() = TRUE;
	canvas_item.property_arrow_shape_a() = 11.0;
	canvas_item.property_arrow_shape_b() = 0.0;
	canvas_item.property_arrow_shape_c() = 9.0;

	canvas_item.set_data ("cursor", this);
	canvas_item.Event.connect (sigc::bind (sigc::mem_fun (ed, callbck), &canvas_item));
	current_frame = 1; /* force redraw at 0 */
}

EditorCursor::~EditorCursor ()
{
}

void
EditorCursor::set_position (framepos_t frame)
{
	PositionChanged (frame);

	double new_pos = editor.frame_to_unit (frame);

	if (new_pos != canvas_item.x0 ()) {
		canvas_item.set_x0 (new_pos);
		canvas_item.set_x1 (new_pos);
	}
	
	current_frame = frame;
}

void
EditorCursor::set_length (double units)
{
	length = units;
	canvas_item.set_y1 (canvas_item.y1 () + length);
}

void
EditorCursor::set_y_axis (double position)
{
	canvas_item.set_y0 (position);
	canvas_item.set_y1 (position + length);
}
