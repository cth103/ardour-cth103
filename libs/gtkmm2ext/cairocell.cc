/*
  Copyright (C) 2011 Paul Davis

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

#include <algorithm>
#include <cmath>

#include "gtkmm2ext/cairocell.h"
#include "gtkmm2ext/utils.h"

using std::string;
using std::map;
using namespace Gtkmm2ext;

CairoCell::CairoCell ()
	: _visible (true)
	, _xpad (5)
{
	bbox.x = 0;
	bbox.y = 0;
	bbox.width = 0;
	bbox.height = 0;
}

CairoTextCell::CairoTextCell (uint32_t wc)
	: _width_chars (wc)
{
}

void
CairoTextCell::render (Cairo::RefPtr<Cairo::Context>& context)
{
	if (!_visible || _width_chars == 0) {
		return;
	}

	context->move_to (bbox.x, bbox.y);
	pango_cairo_update_layout (context->cobj(), layout->gobj());
	pango_cairo_show_layout (context->cobj(), layout->gobj());
}

void
CairoTextCell::set_size (Glib::RefPtr<Pango::Context>& context, const Pango::FontDescription& font)
{
	layout = Pango::Layout::create (context);
	layout->set_font_description (font);

	Pango::FontMetrics metrics = context->get_metrics (font);

	bbox.width = (_width_chars * metrics.get_approximate_digit_width ()) / PANGO_SCALE;
	bbox.height = (metrics.get_ascent() + metrics.get_descent()) / PANGO_SCALE;
}

CairoCell*
CairoEditableText::get_cell (uint32_t id)
{
	CellMap::iterator i = cells.find (id);
	if (i == cells.end()) {
		return 0;
	}
	return i->second;
}

CairoEditableText::CairoEditableText ()
	: editing_id (0)
	, editing_pos (0)
	, width (0)
	, max_cell_height (0)
	, height (0)
	, corner_radius (24)
	, xpad (10)
	, ypad (5)
{
	add_events (Gdk::POINTER_MOTION_HINT_MASK | Gdk::SCROLL_MASK | Gdk::KEY_PRESS_MASK | Gdk::KEY_RELEASE_MASK |
	            Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
	set_flags (Gtk::CAN_FOCUS);
	set_can_default (true);
	set_receives_default (true);
}

bool
CairoEditableText::on_focus_in_event (GdkEventFocus* ev)
{
	return false;
}

bool
CairoEditableText::on_focus_out_event (GdkEventFocus* ev)
{
	if (editing_id) {
		CairoCell* cell = get_cell (editing_id);
		queue_draw_cell (cell);
		editing_id = 0;
		editing_pos = 0;
	}

	return false;
}

void
CairoEditableText::add_cell (uint32_t id, CairoCell* cell)
{
	if (id > 0) {
		Glib::RefPtr<Pango::Context> context = get_pango_context ();
		cell->set_size (context, font);

		cells[id] = cell; /* we own it */
	}
}

void
CairoEditableText::set_text (uint32_t id, const string& text)
{
	CellMap::iterator i = cells.find (id);

	if (i == cells.end()) {
		return;
	}

	CairoTextCell* textcell = dynamic_cast<CairoTextCell*> (i->second);

	if (textcell) {
		set_text (textcell, text);
	}
}

void
CairoEditableText::set_text (CairoTextCell* cell, const string& text)
{
	cell->set_text (text);
	queue_draw_cell (cell);
}

bool
CairoEditableText::on_expose_event (GdkEventExpose* ev)
{
	Cairo::RefPtr<Cairo::Context> context = get_window()->create_cairo_context();

	if (cells.empty()) {
		return true;
	}

	context->rectangle (ev->area.x, ev->area.y, ev->area.width, ev->area.height);
	context->clip ();

	context->set_source_rgba (bg_r, bg_g, bg_b, bg_a);
	rounded_rectangle (context, 0, 0, width, height, corner_radius);
	context->fill ();

	for (CellMap::iterator i = cells.begin(); i != cells.end(); ++i) {

		uint32_t id = i->first;
		CairoCell* cell = i->second;

		/* is cell inside the expose area?
		 */

		if (cell->intersects (ev->area)) {

			if (id == editing_id) {
				context->set_source_rgba (edit_r, edit_b, edit_g, edit_a);
			} else {
				context->set_source_rgba (r, g, b, a);
			}

			cell->render (context);
		}
	}
	return true;
}

void
CairoEditableText::queue_draw_cell (CairoCell* cell)
{
	Glib::RefPtr<Gdk::Window> win = get_window();

	if (!win) {
		return;
	}

	Gdk::Rectangle r;

	r.set_x (cell->x());
	r.set_y (cell->y());
	r.set_width (cell->width());
	r.set_height (cell->height());

	Gdk::Region rg (r);
	win->invalidate_region (rg, true);
}

CairoCell*
CairoEditableText::find_cell (uint32_t x, uint32_t y, uint32_t& id)
{
	for (CellMap::iterator i = cells.begin(); i != cells.end(); ++i) {
		if (i->second->covers (x, y)) {
			id = i->first;
			return i->second;
		}
	}

	return 0;
}

bool
CairoEditableText::on_button_press_event (GdkEventButton* ev)
{
	uint32_t id;
	CairoCell* cell;

	if (editing_id) {
		cell = get_cell (editing_id);
		/* redraw the old cell */
		queue_draw_cell (cell);
	}

	cell = find_cell (ev->x, ev->y, id);

	if (!cell) {
		editing_id = 0;
		return false;
	}

	grab_focus ();
	editing_id = id;
	editing_pos = 0;

	/* redraw the new cell (maybe the same as the old - no real cost) */
	queue_draw_cell (cell);

	return true;
}

bool
CairoEditableText::on_button_release_event (GdkEventButton* ev)
{
	return true;
}

bool
CairoEditableText::on_key_press_event (GdkEventKey* ev)
{
	if (!editing_id) {
		return true;
	}

	bool commit_change = false;

	CairoCell* cell = get_cell (editing_id);

	if (!cell) {
		return true;
	}

	CairoTextCell* text_cell = dynamic_cast<CairoTextCell*> (cell);

	if (!text_cell) {
		return true;
	}

	string txt = text_cell->get_text ();

	switch (ev->keyval) {
	case GDK_Tab:
		queue_draw_cell (cell);
		edit_next_cell ();
		break;

	case GDK_0:
	case GDK_KP_0:
		txt[editing_pos] = '0';
		commit_change = true;
		break;
	case GDK_1:
	case GDK_KP_1:
		txt[editing_pos] = '1';
		commit_change = true;
		break;
	case GDK_2:
	case GDK_KP_2:
		txt[editing_pos] = '2';
		commit_change = true;
		break;
	case GDK_3:
	case GDK_KP_3:
		txt[editing_pos] = '3';
		commit_change = true;
		break;
	case GDK_4:
	case GDK_KP_4:
		txt[editing_pos] = '4';
		commit_change = true;
		break;
	case GDK_5:
	case GDK_KP_5:
		txt[editing_pos] = '5';
		commit_change = true;
		break;
	case GDK_6:
	case GDK_KP_6:
		txt[editing_pos] = '6';
		commit_change = true;
		break;
	case GDK_7:
	case GDK_KP_7:
		txt[editing_pos] = '7';
		commit_change = true;
		break;
	case GDK_8:
	case GDK_KP_8:
		txt[editing_pos] = '8';
		commit_change = true;
		break;
	case GDK_9:
	case GDK_KP_9:
		txt[editing_pos] = '9';
		commit_change = true;
		break;

	case GDK_Right:
		if (editing_pos < text_cell->width_chars() - 1) {
			editing_pos++;
		}
		break;

	case GDK_Left:
		if (editing_pos > 0) {
			editing_pos--;
		}
		break;

	default:
		break;
	}

	if (commit_change) {
		set_text (text_cell, txt);

		if (++editing_pos >= text_cell->width_chars()) {
			edit_next_cell ();
		}
	}


	return true;
}

void
CairoEditableText::edit_next_cell ()
{
	CairoCell* next;
	CairoTextCell* next_text;
	uint32_t next_id = editing_id + 1;

	while (true) {
		next = get_cell (next_id);

		if (!next || !next->visible() || (next_text = dynamic_cast<CairoTextCell*> (next)) != 0) {
			break;
		}

		next_id += 1;
	}

	if (next) {
		editing_id = next_id;
		editing_pos = 0;
		queue_draw_cell (next_text);
	} else {
		editing_id = 0;
		editing_pos = 0;
	}
}

bool
CairoEditableText::on_key_release_event (GdkEventKey* ev)
{
	return true;
}

void
CairoEditableText::on_size_request (GtkRequisition* req)
{
	double x = 0;

	max_cell_height = 0;

	x = xpad;

	for (CellMap::iterator i = cells.begin(); i != cells.end(); ++i) {
		CairoCell* cell = i->second;

		if (cell->visible()) {
			cell->set_position (x, ypad);
		}

		x += cell->width() + cell->xpad();
		max_cell_height = std::max ((double) cell->height(), max_cell_height);
	}

	x += xpad;

	req->width = x;
	req->height = max_cell_height + (ypad * 2);
}

void
CairoEditableText::on_size_allocate (Gtk::Allocation& alloc)
{
	Misc::on_size_allocate (alloc);

	width = alloc.get_width();
	height = alloc.get_height();
}