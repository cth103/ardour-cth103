/*
    Copyright (C) 2002 Paul Davis

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

#include <cmath>
#include <climits>
#include <string.h>

#include <cairo.h>
#include <gtkmm/menu.h>

#include "gtkmm2ext/gtk_ui.h"

#include "pbd/error.h"
#include "pbd/cartesian.h"
#include "ardour/panner.h"
#include "ardour/panner_shell.h"
#include "ardour/pannable.h"
#include "ardour/speakers.h"

#include "panner2d.h"
#include "keyboard.h"
#include "gui_thread.h"
#include "utils.h"
#include "public_editor.h"

#include "i18n.h"

using namespace std;
using namespace Gtk;
using namespace ARDOUR;
using namespace PBD;
using Gtkmm2ext::Keyboard;

static const int large_size_threshold = 100;
static const int large_border_width = 25;
static const int small_border_width = 8;

Panner2d::Target::Target (const AngularVector& a, const char *txt)
	: position (a)
	, text (txt)
	, _selected (false)
{
}

Panner2d::Target::~Target ()
{
}

void
Panner2d::Target::set_text (const char* txt)
{
	text = txt;
}

Panner2d::Panner2d (boost::shared_ptr<PannerShell> p, int32_t h)
	: panner_shell (p)
        , position (AngularVector (0.0, 0.0), "")
        , width (0)
        , height (h)
        , last_width (0)
{
	panner_shell->Changed.connect (connections, invalidator (*this), boost::bind (&Panner2d::handle_state_change, this), gui_context());

        panner_shell->pannable()->pan_azimuth_control->Changed.connect (connections, invalidator(*this), boost::bind (&Panner2d::handle_position_change, this), gui_context());
        panner_shell->pannable()->pan_width_control->Changed.connect (connections, invalidator(*this), boost::bind (&Panner2d::handle_position_change, this), gui_context());

	drag_target = 0;
	set_events (Gdk::BUTTON_PRESS_MASK|Gdk::BUTTON_RELEASE_MASK|Gdk::POINTER_MOTION_MASK);

        handle_position_change ();
}

Panner2d::~Panner2d()
{
	for (Targets::iterator i = speakers.begin(); i != speakers.end(); ++i) {
		delete *i;
	}
}

void
Panner2d::reset (uint32_t n_inputs)
{
        uint32_t nouts = panner_shell->panner()->out().n_audio();

	/* signals */

	while (signals.size() < n_inputs) {
		add_signal ("", AngularVector());
	}

	if (signals.size() > n_inputs) {
		for (uint32_t i = signals.size(); i < n_inputs; ++i) {
			delete signals[i];
		}

		signals.resize (n_inputs);
	}

        label_signals ();

        for (uint32_t i = 0; i < n_inputs; ++i) {
                signals[i]->position = panner_shell->panner()->signal_position (i);
        }

	/* add all outputs */

	while (speakers.size() < nouts) {
		add_speaker (AngularVector());
	}

	if (speakers.size() > nouts) {
		for (uint32_t i = nouts; i < speakers.size(); ++i) {
			delete speakers[i];
		}

		speakers.resize (nouts);
	}

	for (Targets::iterator x = speakers.begin(); x != speakers.end(); ++x) {
		(*x)->visible = false;
	}

        vector<Speaker>& the_speakers (panner_shell->panner()->get_speakers()->speakers());

	for (uint32_t n = 0; n < nouts; ++n) {
		char buf[16];

		snprintf (buf, sizeof (buf), "%d", n+1);
		speakers[n]->set_text (buf);
		speakers[n]->position = the_speakers[n].angles();
		speakers[n]->visible = true;
	}

	queue_draw ();
}

void
Panner2d::on_size_allocate (Gtk::Allocation& alloc)
{
  	width = alloc.get_width();
  	height = alloc.get_height();

        if (height > large_size_threshold) {
                border = large_border_width;
        } else {
                border = small_border_width;
        }

        radius = min (width, height);
        radius -= border;
        radius /= 2;

        hoffset = max ((double) (width - height), border);
        voffset = max ((double) (height - width), border);

        hoffset /= 2.0;
        voffset /= 2.0;

	DrawingArea::on_size_allocate (alloc);
}

int
Panner2d::add_signal (const char* text, const AngularVector& a)
{
	Target* signal = new Target (a, text);
	signals.push_back (signal);
	signal->visible = true;

	return 0;
}

int
Panner2d::add_speaker (const AngularVector& a)
{
	Target* speaker = new Target (a, "");
	speakers.push_back (speaker);
	speaker->visible = true;
	queue_draw ();

	return speakers.size() - 1;
}

void
Panner2d::handle_state_change ()
{
	queue_draw ();
}

void
Panner2d::label_signals ()
{
        double w = panner_shell->pannable()->pan_width_control->get_value();
        uint32_t sz = signals.size();

	switch (sz) {
	case 0:
		break;

	case 1:
		signals[0]->set_text ("");
		break;

	case 2:
                if (w  >= 0.0) {
                        signals[0]->set_text ("R");
                        signals[1]->set_text ("L");
                } else {
                        signals[0]->set_text ("L");
                        signals[1]->set_text ("R");
                }
		break;

	default:
		for (uint32_t i = 0; i < sz; ++i) {
			char buf[64];
                        if (w >= 0.0) {
                                snprintf (buf, sizeof (buf), "%" PRIu32, i + 1);
                        } else {
                                snprintf (buf, sizeof (buf), "%" PRIu32, sz - i);
                        }
			signals[i]->set_text (buf);
		}
		break;
	}
}

void
Panner2d::handle_position_change ()
{
	uint32_t n;
        double w = panner_shell->pannable()->pan_width_control->get_value();

        position.position = AngularVector (panner_shell->pannable()->pan_azimuth_control->get_value() * 360.0, 0.0);

        for (uint32_t i = 0; i < signals.size(); ++i) {
                signals[i]->position = panner_shell->panner()->signal_position (i);
        }

        if (w * last_width <= 0) {
                /* changed sign */
                label_signals ();
        }

        last_width = w;

        vector<Speaker>& the_speakers (panner_shell->panner()->get_speakers()->speakers());

	for (n = 0; n < speakers.size(); ++n) {
		speakers[n]->position = the_speakers[n].angles();
	}

	queue_draw ();
}

void
Panner2d::move_signal (int which, const AngularVector& a)
{
	if (which >= int (speakers.size())) {
		return;
	}

	speakers[which]->position = a;
	queue_draw ();
}

Panner2d::Target *
Panner2d::find_closest_object (gdouble x, gdouble y, bool& is_signal)
{
	Target *closest = 0;
	Target *candidate;
	float distance;
	float best_distance = FLT_MAX;
        CartesianVector c;

        /* start with the position itself
         */

        position.position.cartesian (c);
        cart_to_gtk (c);
        best_distance = sqrt ((c.x - x) * (c.x - x) +
                         (c.y - y) * (c.y - y));
        closest = &position;

	for (Targets::const_iterator i = signals.begin(); i != signals.end(); ++i) {
		candidate = *i;

		candidate->position.cartesian (c);
		cart_to_gtk (c);

		distance = sqrt ((c.x - x) * (c.x - x) +
		                 (c.y - y) * (c.y - y));

                if (distance < best_distance) {
			closest = candidate;
			best_distance = distance;
		}
	}

        is_signal = true;

        if (height > large_size_threshold) {
                /* "big" */
                if (best_distance > 30) { // arbitrary
                        closest = 0;
                }
        } else {
                /* "small" */
                if (best_distance > 10) { // arbitrary
                        closest = 0;
                }
        }

        /* if we didn't find a signal close by, check the speakers */

        if (!closest) {
                for (Targets::const_iterator i = speakers.begin(); i != speakers.end(); ++i) {
                        candidate = *i;

                        candidate->position.cartesian (c);
                        cart_to_gtk (c);

                        distance = sqrt ((c.x - x) * (c.x - x) +
                                         (c.y - y) * (c.y - y));

                        if (distance < best_distance) {
                                closest = candidate;
                                best_distance = distance;
                        }
                }

                if (height > large_size_threshold) {
                        /* "big" */
                        if (best_distance < 30) { // arbitrary
                                is_signal = false;
                        } else {
                                closest = 0;
                        }
                } else {
                        /* "small" */
                        if (best_distance < 10) { // arbitrary
                                is_signal = false;
                        } else {
                                closest = 0;
                        }
                }
        }

	return closest;
}

bool
Panner2d::on_motion_notify_event (GdkEventMotion *ev)
{
	gint x, y;
	GdkModifierType state;

	if (ev->is_hint) {
		gdk_window_get_pointer (ev->window, &x, &y, &state);
	} else {
		x = (int) floor (ev->x);
		y = (int) floor (ev->y);
		state = (GdkModifierType) ev->state;
	}

        if (ev->state & (GDK_BUTTON1_MASK|GDK_BUTTON2_MASK)) {
                did_move = true;
        }

	return handle_motion (x, y, state);
}

bool
Panner2d::on_expose_event (GdkEventExpose *event)
{
        CartesianVector c;
	cairo_t* cr;
        bool small = (height <= large_size_threshold);
        const double diameter = radius*2.0;

	cr = gdk_cairo_create (get_window()->gobj());

        /* background */

	cairo_rectangle (cr, event->area.x, event->area.y, event->area.width, event->area.height);
	if (!panner_shell->bypassed()) {
		cairo_set_source_rgba (cr, 0.1, 0.1, 0.1, 1.0);
	} else {
		cairo_set_source_rgba (cr, 0.1, 0.1, 0.1, 0.2);
	}
	cairo_fill_preserve (cr);
	cairo_clip (cr);

        /* offset to give us some border */

        cairo_translate (cr, hoffset, voffset);

	cairo_set_line_width (cr, 1.0);

	/* horizontal line of "crosshairs" */

        cairo_set_source_rgba (cr, 0.282, 0.517, 0.662, 1.0);
	cairo_move_to (cr, 0.0, radius);
	cairo_line_to (cr, diameter, radius);
	cairo_stroke (cr);

	/* vertical line of "crosshairs" */

	cairo_move_to (cr, radius, 0);
	cairo_line_to (cr, radius, diameter);
	cairo_stroke (cr);

	/* the circle on which signals live */

	cairo_set_line_width (cr, 2.0);
        cairo_set_source_rgba (cr, 0.517, 0.772, 0.882, 1.0);
	cairo_arc (cr, radius, radius, radius, 0.0, 2.0 * M_PI);
	cairo_stroke (cr);

	/* 3 other circles of smaller diameter circle on which signals live */

	cairo_set_line_width (cr, 1.0);
        cairo_set_source_rgba (cr, 0.282, 0.517, 0.662, 1.0);
	cairo_arc (cr, radius, radius, radius * 0.75, 0, 2.0 * M_PI);
	cairo_stroke (cr);
        cairo_set_source_rgba (cr, 0.282, 0.517, 0.662, 0.85);
	cairo_arc (cr, radius, radius, radius * 0.50, 0, 2.0 * M_PI);
	cairo_stroke (cr);
	cairo_arc (cr, radius, radius, radius * 0.25, 0, 2.0 * M_PI);
	cairo_stroke (cr);

        if (signals.size() > 1) {
                /* arc to show "diffusion" */

                double width_angle = fabs (panner_shell->pannable()->pan_width_control->get_value()) * 2 * M_PI;
                double position_angle = (2 * M_PI) - panner_shell->pannable()->pan_azimuth_control->get_value() * 2 * M_PI;

                cairo_save (cr);
                cairo_translate (cr, radius, radius);
                cairo_rotate (cr, position_angle - (width_angle/2.0));
                cairo_move_to (cr, 0, 0);
                cairo_arc_negative (cr, 0, 0, radius, width_angle, 0.0);
                cairo_close_path (cr);
                if (panner_shell->pannable()->pan_width_control->get_value() >= 0.0) {
                        /* normal width */
                        cairo_set_source_rgba (cr, 0.282, 0.517, 0.662, 0.45);
                } else {
                        /* inverse width */
                        cairo_set_source_rgba (cr, 1.0, 0.419, 0.419, 0.45);
                }
                cairo_fill (cr);
                cairo_restore (cr);
        }

	if (!panner_shell->bypassed()) {

		double arc_radius;

		cairo_select_font_face (cr, "sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);

		if (small) {
			arc_radius = 4.0;
		} else {
			cairo_set_font_size (cr, 10);
			arc_radius = 12.0;
		}

                /* signals */

                if (signals.size() > 1) {
                        for (Targets::iterator i = signals.begin(); i != signals.end(); ++i) {
                                Target* signal = *i;

                                if (signal->visible) {

                                        signal->position.cartesian (c);
                                        cart_to_gtk (c);

                                        cairo_new_path (cr);
                                        cairo_arc (cr, c.x, c.y, arc_radius, 0, 2.0 * M_PI);
                                        cairo_set_source_rgba (cr, 0.282, 0.517, 0.662, 0.85);
                                        cairo_fill_preserve (cr);
                                        cairo_set_source_rgba (cr, 0.517, 0.772, 0.882, 1.0);
                                        cairo_stroke (cr);

                                        if (!small && !signal->text.empty()) {
                                                cairo_set_source_rgb (cr, 0.517, 0.772, 0.882);
                                                /* the +/- adjustments are a hack to try to center the text in the circle */
                                                if (small) {
                                                        cairo_move_to (cr, c.x - 1, c.y + 1);
                                                } else {
                                                        cairo_move_to (cr, c.x - 4, c.y + 4);
                                                }
                                                cairo_show_text (cr, signal->text.c_str());
                                        }
                                }
                        }
                }

                /* speakers */

		int n = 0;

		for (Targets::iterator i = speakers.begin(); i != speakers.end(); ++i) {
			Target *speaker = *i;
			char buf[256];
			++n;

			if (speaker->visible) {

				CartesianVector c;

				speaker->position.cartesian (c);
				cart_to_gtk (c);

				snprintf (buf, sizeof (buf), "%d", n);

                                /* stroke out a speaker shape */

                                cairo_move_to (cr, c.x, c.y);
                                cairo_save (cr);
                                cairo_rotate (cr, -(speaker->position.azi/360.0) * (2.0 * M_PI));
                                if (small) {
                                        cairo_scale (cr, 0.8, 0.8);
                                } else {
                                        cairo_scale (cr, 1.2, 1.2);
                                }
                                cairo_rel_line_to (cr, 4, -2);
                                cairo_rel_line_to (cr, 0, -7);
                                cairo_rel_line_to (cr, 5, +5);
                                cairo_rel_line_to (cr, 5, 0);
                                cairo_rel_line_to (cr, 0, 5);
                                cairo_rel_line_to (cr, -5, 0);
                                cairo_rel_line_to (cr, -5, +5);
                                cairo_rel_line_to (cr, 0, -7);
                                cairo_close_path (cr);
				cairo_set_source_rgba (cr, 0.282, 0.517, 0.662, 1.0);
				cairo_fill (cr);
                                cairo_restore (cr);

                                if (!small) {
                                        cairo_set_font_size (cr, 16);

                                        /* move the text in just a bit */

                                        AngularVector textpos (speaker->position.azi, speaker->position.ele, 0.85);
                                        textpos.cartesian (c);
                                        cart_to_gtk (c);
                                        cairo_move_to (cr, c.x, c.y);
                                        cairo_show_text (cr, buf);
                                }

			}
		}

                /* draw position */

                position.position.cartesian (c);
                cart_to_gtk (c);

                cairo_new_path (cr);
                cairo_arc (cr, c.x, c.y, arc_radius, 0, 2.0 * M_PI);
                cairo_set_source_rgba (cr, 1.0, 0.419, 0.419, 0.85);
                cairo_fill_preserve (cr);
                cairo_set_source_rgba (cr, 1.0, 0.905, 0.905, 0.85);
                cairo_stroke (cr);
	}

	cairo_destroy (cr);

	return true;
}

bool
Panner2d::on_button_press_event (GdkEventButton *ev)
{
	GdkModifierType state;
        int x;
        int y;
        bool is_signal;

	if (ev->type == GDK_2BUTTON_PRESS && ev->button == 1) {
		return false;
	}

        did_move = false;

	switch (ev->button) {
	case 1:
	case 2:
                x = ev->x - border;
                y = ev->y - border;

		if ((drag_target = find_closest_object (x, y, is_signal)) != 0) {
                        if (!is_signal) {
                                panner_shell->panner()->set_position (drag_target->position.azi/360.0);
                                drag_target = 0;
                        } else {
                                drag_target->set_selected (true);
                        }
		}

		drag_x = ev->x;
		drag_y = ev->y;
		state = (GdkModifierType) ev->state;

		return handle_motion (drag_x, drag_y, state);
		break;

	default:
		break;
	}

	return false;
}

bool
Panner2d::on_button_release_event (GdkEventButton *ev)
{
	gint x, y;
	GdkModifierType state;
	bool ret = false;

	switch (ev->button) {
	case 1:
		x = (int) floor (ev->x);
		y = (int) floor (ev->y);
		state = (GdkModifierType) ev->state;
                ret = handle_motion (x, y, state);
		drag_target = 0;
		break;

	case 2:
		x = (int) floor (ev->x);
		y = (int) floor (ev->y);
		state = (GdkModifierType) ev->state;

		if (Keyboard::modifier_state_contains (state, Keyboard::TertiaryModifier)) {
			toggle_bypass ();
			ret = true;
		} else {
			ret = handle_motion (x, y, state);
		}

		drag_target = 0;
		break;

	case 3:
		break;

	}

	return ret;
}

gint
Panner2d::handle_motion (gint evx, gint evy, GdkModifierType state)
{
	if (drag_target == 0) {
		return false;
	}

	if ((state & (GDK_BUTTON1_MASK|GDK_BUTTON2_MASK)) == 0) {
		return false;
	}


	if (state & GDK_BUTTON1_MASK && !(state & GDK_BUTTON2_MASK)) {
		CartesianVector c;
		bool need_move = false;

		drag_target->position.cartesian (c);
		cart_to_gtk (c);

		if ((evx != c.x) || (evy != c.y)) {
			need_move = true;
		}

		if (need_move) {
			CartesianVector cp (evx, evy, 0.0);
                        AngularVector av;

			/* canonicalize position and then clamp to the circle */

			gtk_to_cart (cp);
			clamp_to_circle (cp.x, cp.y);

			/* generate an angular representation of the current mouse position */

			cp.angular (av);

                        if (drag_target == &position) {
                                double degree_fract = av.azi / 360.0;
                                panner_shell->panner()->set_position (degree_fract);
                        }
		}
	}

	return true;
}

bool
Panner2d::on_scroll_event (GdkEventScroll* ev)
{
        switch (ev->direction) {
        case GDK_SCROLL_UP:
        case GDK_SCROLL_RIGHT:
                panner_shell->panner()->set_position (panner_shell->pannable()->pan_azimuth_control->get_value() - 1.0/360.0);
                break;

        case GDK_SCROLL_DOWN:
        case GDK_SCROLL_LEFT:
                panner_shell->panner()->set_position (panner_shell->pannable()->pan_azimuth_control->get_value() + 1.0/360.0);
                break;
        }
        return true;
}

void
Panner2d::cart_to_gtk (CartesianVector& c) const
{
	/* cartesian coordinate space:
   	      center = 0.0
              dimension = 2.0 * 2.0
              increasing y moves up
              so max values along each axis are -1..+1

	   GTK uses a coordinate space that is:
  	      top left = 0.0
              dimension = (radius*2.0) * (radius*2.0)
              increasing y moves down
	*/
        const double diameter = radius*2.0;

        c.x = diameter * ((c.x + 1.0) / 2.0);
        /* extra subtraction inverts the y-axis to match "increasing y moves down" */
        c.y = diameter - (diameter * ((c.y + 1.0) / 2.0));
}

void
Panner2d::gtk_to_cart (CartesianVector& c) const
{
        const double diameter = radius*2.0;
	c.x = ((c.x / diameter) * 2.0) - 1.0;
        c.y = (((diameter - c.y) / diameter) * 2.0) - 1.0;
}

void
Panner2d::clamp_to_circle (double& x, double& y)
{
	double azi, ele;
	double z = 0.0;
        double l;

	PBD::cartesian_to_spherical (x, y, z, azi, ele, l);
	PBD::spherical_to_cartesian (azi, ele, 1.0, x, y, z);
}

void
Panner2d::toggle_bypass ()
{
	panner_shell->set_bypassed (!panner_shell->bypassed());
}

Panner2dWindow::Panner2dWindow (boost::shared_ptr<PannerShell> p, int32_t h, uint32_t inputs)
	: ArdourWindow (_("Panner (2D)"))
        , widget (p, h)
	, bypass_button (_("Bypass"))
{
	widget.set_name ("MixerPanZone");

	set_title (_("Panner"));
	widget.set_size_request (h, h);

        bypass_button.signal_toggled().connect (sigc::mem_fun (*this, &Panner2dWindow::bypass_toggled));

	button_box.set_spacing (6);
	button_box.pack_start (bypass_button, false, false);

	spinner_box.set_spacing (6);
	left_side.set_spacing (6);

	left_side.pack_start (button_box, false, false);
	left_side.pack_start (spinner_box, false, false);

	bypass_button.show ();
	button_box.show ();
	spinner_box.show ();
	left_side.show ();

	hpacker.set_spacing (6);
	hpacker.set_border_width (12);
	hpacker.pack_start (widget, false, false);
	hpacker.pack_start (left_side, false, false);
	hpacker.show ();

	add (hpacker);
	reset (inputs);
	widget.show ();
}

void
Panner2dWindow::reset (uint32_t n_inputs)
{
	widget.reset (n_inputs);

#if 0
	while (spinners.size() < n_inputs) {
		// spinners.push_back (new Gtk::SpinButton (widget.azimuth (spinners.size())));
		//spinner_box.pack_start (*spinners.back(), false, false);
		//spinners.back()->set_digits (4);
		spinners.back()->show ();
	}

	while (spinners.size() > n_inputs) {
		spinner_box.remove (*spinners.back());
		delete spinners.back();
		spinners.erase (--spinners.end());
	}
#endif
}

void
Panner2dWindow::bypass_toggled ()
{
        bool view = bypass_button.get_active ();
        bool model = widget.get_panner_shell()->bypassed ();

        if (model != view) {
                widget.get_panner_shell()->set_bypassed (view);
        }
}

bool
Panner2dWindow::on_key_press_event (GdkEventKey* event)
{
        return relay_key_press (event, &PublicEditor::instance());
}

bool
Panner2dWindow::on_key_release_event (GdkEventKey*)
{
        return true;
}
