/*
    Copyright (C) 2009 Paul Davis

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

#ifndef __gtk2_ardour_cairo_widget_h__
#define __gtk2_ardour_cairo_widget_h__

#include <gtkmm/eventbox.h>
#include "gtkmm2ext/widget_state.h"

/** A parent class for widgets that are rendered using Cairo.
 */

class CairoWidget : public Gtk::EventBox
{
public:
	CairoWidget ();
	virtual ~CairoWidget ();

	void set_dirty ();

	Gtkmm2ext::ActiveState active_state() const { return _active_state; }
	Gtkmm2ext::VisualState visual_state() const { return _visual_state; }
	virtual void set_active_state (Gtkmm2ext::ActiveState);
	virtual void set_visual_state (Gtkmm2ext::VisualState);
	virtual void unset_active_state () { set_active_state (Gtkmm2ext::ActiveState (0)); }
	virtual void unset_visual_state () { set_visual_state (Gtkmm2ext::VisualState (0)); }

	sigc::signal<void> StateChanged;

protected:
	/** Render the widget to the given Cairo context */
	virtual void render (cairo_t *) = 0;
	virtual bool on_expose_event (GdkEventExpose *);
	void on_size_allocate (Gtk::Allocation &);
	Gdk::Color get_parent_bg ();

	int _width; ///< pixmap width
	int _height; ///< pixmap height
	Gtkmm2ext::ActiveState _active_state;
	Gtkmm2ext::VisualState _visual_state;

private:
	GdkPixmap* _pixmap; ///< our pixmap
};

#endif
