/*
    Copyright (C) 2006 Paul Davis 

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

#ifndef __gtkmm2ext_pixfader_h__
#define __gtkmm2ext_pixfader_h__

#include <cmath>
#include <stdint.h>

#include <gtkmm/drawingarea.h>
#include <gtkmm/adjustment.h>
#include <gdkmm.h>

namespace Gtkmm2ext {

class PixFader : public Gtk::DrawingArea
{
  public:
	PixFader (Glib::RefPtr<Gdk::Pixbuf> belt_image, Gtk::Adjustment& adjustment, int orientation, int);
	virtual ~PixFader ();

	void set_fader_length (int);
        void set_border_colors (uint32_t rgba_left, uint32_t rgba_right);

  protected:
	Gtk::Adjustment& adjustment;

	void on_size_request (GtkRequisition*);

	bool on_expose_event (GdkEventExpose*);
	bool on_button_press_event (GdkEventButton*);
	bool on_button_release_event (GdkEventButton*);
	bool on_motion_notify_event (GdkEventMotion*);
	bool on_scroll_event (GdkEventScroll* ev);
	bool on_enter_notify_event (GdkEventCrossing* ev);
	bool on_leave_notify_event (GdkEventCrossing* ev);

	enum Orientation {
		VERT=1,
		HORIZ=2,
	};

  private:	
        Cairo::RefPtr<Cairo::Context> belt_context;
        Cairo::RefPtr<Cairo::ImageSurface> belt_surface;
        Glib::RefPtr<Gdk::Pixbuf> pixbuf;
	int span, girth;
	int _orien;
        float left_r;
        float left_g;
        float left_b;
        float right_r;
        float right_g;
        float right_b;

	GdkRectangle view;

	GdkWindow* grab_window;
	double grab_loc;
	double grab_start;
	int last_drawn;
	bool dragging;
	float default_value;
	int unity_loc;

	void adjustment_changed ();
	int display_span ();
	void set_adjustment_from_event (GdkEventButton *);

	static int fine_scale_modifier;
	static int extra_fine_scale_modifier;
};


} /* namespace */

 #endif /* __gtkmm2ext_pixfader_h__ */
