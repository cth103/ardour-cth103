/*
    Copyright (C) 2001-2006 Paul Davis

    This program is free software; you can r>edistribute it and/or modify
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
#include <cassert>
#include <algorithm>

#include <boost/scoped_ptr.hpp>

#include <gtkmm.h>

#include <gtkmm2ext/gtk_ui.h>

#include "ardour/playlist.h"
#include "ardour/audioregion.h"
#include "ardour/audiosource.h"
#include "ardour/profile.h"
#include "ardour/session.h"

#include "pbd/memento_command.h"
#include "pbd/stacktrace.h"

#include "evoral/Curve.hpp"

#include "canvas/rectangle.h"
#include "canvas/polygon.h"
#include "canvas/pixbuf.h"

#include "streamview.h"
#include "audio_region_view.h"
#include "audio_time_axis.h"
#include "public_editor.h"
#include "audio_region_editor.h"
#include "audio_streamview.h"
#include "region_gain_line.h"
#include "control_point.h"
#include "ghostregion.h"
#include "audio_time_axis.h"
#include "utils.h"
#include "rgb_macros.h"
#include "gui_thread.h"
#include "ardour_ui.h"

#include "i18n.h"

#define MUTED_ALPHA 10

using namespace std;
using namespace ARDOUR;
using namespace PBD;
using namespace Editing;
using namespace Canvas;

static const int32_t sync_mark_width = 9;

AudioRegionView::AudioRegionView (Canvas::Group *parent, RouteTimeAxisView &tv, boost::shared_ptr<AudioRegion> r, double spu,
				  Gdk::Color const & basic_color)
	: RegionView (parent, tv, r, spu, basic_color)
	, sync_mark(0)
	, fade_in_shape(0)
	, fade_out_shape(0)
	, fade_in_handle(0)
	, fade_out_handle(0)
	, fade_position_line(0)
	, start_xfade_in (0)
	, start_xfade_out (0)
	, start_xfade_rect (0)
	, end_xfade_in (0)
	, end_xfade_out (0)
	, end_xfade_rect (0)
	, _amplitude_above_axis(1.0)
	, _flags(0)
	, fade_color(0)
{
}

AudioRegionView::AudioRegionView (Canvas::Group *parent, RouteTimeAxisView &tv, boost::shared_ptr<AudioRegion> r, double spu,
				  Gdk::Color const & basic_color, bool recording, TimeAxisViewItem::Visibility visibility)
	: RegionView (parent, tv, r, spu, basic_color, recording, visibility)
	, sync_mark(0)
	, fade_in_shape(0)
	, fade_out_shape(0)
	, fade_in_handle(0)
	, fade_out_handle(0)
	, fade_position_line(0)
	, start_xfade_in (0)
	, start_xfade_out (0)
	, start_xfade_rect (0)
	, end_xfade_in (0)
	, end_xfade_out (0)
	, end_xfade_rect (0)
	, _amplitude_above_axis(1.0)
	, _flags(0)
	, fade_color(0)
{
}

AudioRegionView::AudioRegionView (const AudioRegionView& other, boost::shared_ptr<AudioRegion> other_region)
	: RegionView (other, boost::shared_ptr<Region> (other_region))
	, fade_in_shape(0)
	, fade_out_shape(0)
	, fade_in_handle(0)
	, fade_out_handle(0)
	, fade_position_line(0)
	, start_xfade_in (0)
	, start_xfade_out (0)
	, start_xfade_rect (0)
	, end_xfade_in (0)
	, end_xfade_out (0)
	, end_xfade_rect (0)
	, _amplitude_above_axis (other._amplitude_above_axis)
	, _flags (other._flags)
	, fade_color(0)
{
	Gdk::Color c;
	int r,g,b,a;

	UINT_TO_RGBA (other.fill_color, &r, &g, &b, &a);
	c.set_rgb_p (r/255.0, g/255.0, b/255.0);

	init (c, true);
}

void
AudioRegionView::init (Gdk::Color const & basic_color, bool wfd)
{
	// FIXME: Some redundancy here with RegionView::init.  Need to figure out
	// where order is important and where it isn't...

	RegionView::init (basic_color, wfd);

	XMLNode *node;

	_amplitude_above_axis = 1.0;
	_flags                = 0;

	if ((node = _region->extra_xml ("GUI")) != 0) {
		set_flags (node);
	} else {
		_flags = WaveformVisible;
		store_flags ();
	}

	compute_colors (basic_color);

	create_waves ();

	fade_in_shape = new Canvas::Polygon (group);
	fade_in_shape->set_fill_color (fade_color);
	fade_in_shape->set_data ("regionview", this);

	fade_out_shape = new Canvas::Polygon (group);
	fade_out_shape->set_fill_color (fade_color);
	fade_out_shape->set_data ("regionview", this);

	if (!_recregion) {
		fade_in_handle = new Canvas::Rectangle (group);
		fade_in_handle->set_fill_color (UINT_RGBA_CHANGE_A (fill_color, 0));
		fade_in_handle->set_outline (false);

		fade_in_handle->set_data ("regionview", this);

		fade_out_handle = new Canvas::Rectangle (group);
		fade_out_handle->set_fill_color (UINT_RGBA_CHANGE_A (fill_color, 0));
		fade_out_handle->set_outline (false);

		fade_out_handle->set_data ("regionview", this);
		
		fade_position_line = new Canvas::Line (group);
		fade_position_line->set_outline_color (0xBBBBBBAA);
		fade_position_line->set_y0 (7);
		fade_position_line->set_y1 (_height - TimeAxisViewItem::NAME_HIGHLIGHT_SIZE - 1);
		fade_position_line->hide();
	}

	setup_fade_handle_positions ();

	if (!trackview.session()->config.get_show_region_fades()) {
		set_fade_visibility (false);
	}

	const string line_name = _region->name() + ":gain";

	if (!Profile->get_sae()) {
		gain_line.reset (new AudioRegionGainLine (line_name, *this, *group, audio_region()->envelope()));
	}

	if (Config->get_show_region_gain()) {
		gain_line->show ();
	} else {
		gain_line->hide ();
	}

	gain_line->reset ();

	set_height (trackview.current_height());

	region_muted ();
	region_sync_changed ();

	region_resized (ARDOUR::bounds_change);

	for (vector<GhostRegion*>::iterator i = ghosts.begin(); i != ghosts.end(); ++i) {
		(*i)->set_duration (_region->length() / frames_per_pixel);
	}

	region_locked ();
	envelope_active_changed ();
	fade_in_active_changed ();
	fade_out_active_changed ();

	reset_width_dependent_items (_pixel_width);

	fade_in_shape->Event.connect (sigc::bind (sigc::mem_fun (PublicEditor::instance(), &PublicEditor::canvas_fade_in_event), fade_in_shape, this));
	if (fade_in_handle) {
		fade_in_handle->Event.connect (sigc::bind (sigc::mem_fun (PublicEditor::instance(), &PublicEditor::canvas_fade_in_handle_event), fade_in_handle, this));
	}

	fade_out_shape->Event.connect (sigc::bind (sigc::mem_fun (PublicEditor::instance(), &PublicEditor::canvas_fade_out_event), fade_out_shape, this));

	if (fade_out_handle) {
		fade_out_handle->Event.connect (sigc::bind (sigc::mem_fun (PublicEditor::instance(), &PublicEditor::canvas_fade_out_handle_event), fade_out_handle, this));
	}

	set_colors ();

	/* XXX sync mark drag? */
}

AudioRegionView::~AudioRegionView ()
{
	in_destructor = true;

	RegionViewGoingAway (this); /* EMIT_SIGNAL */

	for (vector<ScopedConnection*>::iterator i = _data_ready_connections.begin(); i != _data_ready_connections.end(); ++i) {
		delete *i;
	}

	for (list<std::pair<framepos_t, Canvas::Line*> >::iterator i = feature_lines.begin(); i != feature_lines.end(); ++i) {
		delete ((*i).second);
	}

	/* all waveviews etc will be destroyed when the group is destroyed */
}

boost::shared_ptr<ARDOUR::AudioRegion>
AudioRegionView::audio_region() const
{
	// "Guaranteed" to succeed...
	return boost::dynamic_pointer_cast<AudioRegion>(_region);
}

void
AudioRegionView::region_changed (const PropertyChange& what_changed)
{
	ENSURE_GUI_THREAD (*this, &AudioRegionView::region_changed, what_changed);
	// cerr << "AudioRegionView::region_changed() called" << endl;

	RegionView::region_changed (what_changed);

	if (what_changed.contains (ARDOUR::Properties::scale_amplitude)) {
		region_scale_amplitude_changed ();
	}
	if (what_changed.contains (ARDOUR::Properties::fade_in)) {
		fade_in_changed ();
	}
	if (what_changed.contains (ARDOUR::Properties::fade_out)) {
		fade_out_changed ();
	}
	if (what_changed.contains (ARDOUR::Properties::fade_in_active)) {
		fade_in_active_changed ();
	}
	if (what_changed.contains (ARDOUR::Properties::fade_out_active)) {
		fade_out_active_changed ();
	}
	if (what_changed.contains (ARDOUR::Properties::envelope_active)) {
		envelope_active_changed ();
	}
	if (what_changed.contains (ARDOUR::Properties::valid_transients)) {
		transients_changed ();
	}
}

void
AudioRegionView::fade_in_changed ()
{
	reset_fade_in_shape ();
}

void
AudioRegionView::fade_out_changed ()
{
	reset_fade_out_shape ();
}

void
AudioRegionView::fade_in_active_changed ()
{
	if (audio_region()->fade_in_active()) {
		/* XXX: make a themable colour */
		fade_in_shape->set_fill_color (RGBA_TO_UINT (45, 45, 45, 90));
		fade_in_shape->set_outline_width (1);
	} else {
		/* XXX: make a themable colour */
		fade_in_shape->set_fill_color (RGBA_TO_UINT (45, 45, 45, 20));
		fade_in_shape->set_outline_width (1);
	}
}

void
AudioRegionView::fade_out_active_changed ()
{
	if (audio_region()->fade_out_active()) {
		/* XXX: make a themable colour */
		fade_out_shape->set_fill_color (RGBA_TO_UINT (45, 45, 45, 90));
		fade_out_shape->set_outline_width (1);
	} else {
		/* XXX: make a themable colour */
		fade_out_shape->set_fill_color (RGBA_TO_UINT (45, 45, 45, 20));
		fade_out_shape->set_outline_width (1);
	}
}


void
AudioRegionView::region_scale_amplitude_changed ()
{
	ENSURE_GUI_THREAD (*this, &AudioRegionView::region_scale_amplitude_changed)

	for (uint32_t n = 0; n < waves.size(); ++n) {
		waves[n]->rebuild ();
	}
}

void
AudioRegionView::region_renamed ()
{
	std::string str = RegionView::make_name ();

	if (audio_region()->speed_mismatch (trackview.session()->frame_rate())) {
		str = string ("*") + str;
	}

	if (_region->muted()) {
		str = string ("!") + str;
	}

	set_item_name (str, this);
	set_name_text (str);
}

void
AudioRegionView::region_resized (const PropertyChange& what_changed)
{
	AudioGhostRegion* agr;

	RegionView::region_resized(what_changed);
	PropertyChange interesting_stuff;

	interesting_stuff.add (ARDOUR::Properties::start);
	interesting_stuff.add (ARDOUR::Properties::length);

	if (what_changed.contains (interesting_stuff)) {

		for (uint32_t n = 0; n < waves.size(); ++n) {
			waves[n]->region_resized ();
			waves[n]->set_region_start (region()->start ());
		}

		for (vector<GhostRegion*>::iterator i = ghosts.begin(); i != ghosts.end(); ++i) {
			if ((agr = dynamic_cast<AudioGhostRegion*>(*i)) != 0) {

				for (vector<WaveView*>::iterator w = agr->waves.begin(); w != agr->waves.end(); ++w) {
					(*w)->region_resized ();
					(*w)->set_region_start (region()->start ());
				}
			}
		}

		/* hide transient lines that extend beyond the region end */

		list<std::pair<framepos_t, Canvas::Line*> >::iterator l;
		for (l = feature_lines.begin(); l != feature_lines.end(); ++l) {
			if (l->first > _region->length() - 1) {
				l->second->hide();
			} else {
				l->second->show();
			}
		}
	}
}

void
AudioRegionView::reset_width_dependent_items (double pixel_width)
{
	RegionView::reset_width_dependent_items(pixel_width);
	assert(_pixel_width == pixel_width);

	if (fade_in_handle) {
		if (pixel_width <= 6.0 || _height < 5.0 || !trackview.session()->config.get_show_region_fades()) {
			fade_in_handle->hide();
			fade_out_handle->hide();
		}
		else {
			fade_in_handle->show();
			fade_out_handle->show();
		}
	}

	AnalysisFeatureList analysis_features = _region->transients();
	AnalysisFeatureList::const_iterator i;

	list<std::pair<framepos_t, Canvas::Line*> >::iterator l;

	for (i = analysis_features.begin(), l = feature_lines.begin(); i != analysis_features.end() && l != feature_lines.end(); ++i, ++l) {

		float x_pos = trackview.editor().frame_to_pixel (*i);

		(*l).first = *i;
		(*l).second->set (
			Canvas::Duple (x_pos, 2.0),
			Canvas::Duple (x_pos, _height - TimeAxisViewItem::NAME_HIGHLIGHT_SIZE - 1)
			);
	}

	reset_fade_shapes ();
}

void
AudioRegionView::region_muted ()
{
	RegionView::region_muted();

	for (uint32_t n=0; n < waves.size(); ++n) {
		if (_region->muted()) {
			waves[n]->set_outline_color (UINT_RGBA_CHANGE_A(ARDOUR_UI::config()->canvasvar_WaveForm.get(), MUTED_ALPHA));
		} else {
			waves[n]->set_outline_color (ARDOUR_UI::config()->canvasvar_WaveForm.get());
		}
	}
}

void
AudioRegionView::setup_fade_handle_positions()
{
	/* position of fade handle offset from the top of the region view */
	double const handle_pos = 2;
	/* height of fade handles */
	double const handle_height = 5;

	if (fade_in_handle) {
		fade_in_handle->set_y0 (handle_pos);
		fade_in_handle->set_y1 (handle_pos + handle_height);
	}

	if (fade_out_handle) {
		fade_out_handle->set_y0 (handle_pos);
		fade_out_handle->set_y1 (handle_pos + handle_height);
	}
}

void
AudioRegionView::set_height (gdouble height)
{
	RegionView::set_height (height);

	uint32_t wcnt = waves.size();

	for (uint32_t n = 0; n < wcnt; ++n) {
		gdouble ht;

		if (height < NAME_HIGHLIGHT_THRESH) {
			ht = ((height - 2 * wcnt) / (double) wcnt);
		} else {
			ht = (((height - 2 * wcnt) - NAME_HIGHLIGHT_SIZE) / (double) wcnt);
		}

		gdouble yoff = n * (ht + 1);

		waves[n]->set_height (ht);
		waves[n]->set_y_position (yoff + 2);
	}

	if (gain_line) {

		if ((height/wcnt) < NAME_HIGHLIGHT_THRESH) {
			gain_line->hide ();
		} else {
			if (Config->get_show_region_gain ()) {
				gain_line->show ();
			}
		}

		gain_line->set_height ((uint32_t) rint (height - NAME_HIGHLIGHT_SIZE) - 2);
	}

	reset_fade_shapes ();

	/* Update hights for any active feature lines */
	list<std::pair<framepos_t, Canvas::Line*> >::iterator l;
	for (l = feature_lines.begin(); l != feature_lines.end(); ++l) {

		float pos_x = trackview.editor().frame_to_pixel((*l).first);

		(*l).second->set (
			Canvas::Duple (pos_x, 2.0),
			Canvas::Duple (pos_x, _height - TimeAxisViewItem::NAME_HIGHLIGHT_SIZE - 1)
			);
	}

	if (fade_position_line) {

		if (height < NAME_HIGHLIGHT_THRESH) {
			fade_position_line->set_y1 (_height - 1);
		} else {
			fade_position_line->set_y1 (_height - TimeAxisViewItem::NAME_HIGHLIGHT_SIZE - 1);
		}
	}

	if (name_pixbuf) {
		name_pixbuf->raise_to_top();
	}
}

void
AudioRegionView::reset_fade_shapes ()
{
	reset_fade_in_shape ();
	reset_fade_out_shape ();
}

void
AudioRegionView::reset_fade_in_shape ()
{
	reset_fade_in_shape_width ((framecnt_t) audio_region()->fade_in()->back()->when);
}

void
AudioRegionView::reset_fade_in_shape_width (framecnt_t width)
{
	if (dragging()) {
		return;
	}

	if (audio_region()->fade_in_is_xfade()) {
		if (fade_in_handle) {
			fade_in_handle->hide ();
			fade_in_shape->hide ();
		}
		redraw_start_xfade ();
		return;
	} else {
		if (start_xfade_in) {
			start_xfade_in->hide ();
			start_xfade_out->hide ();
			start_xfade_rect->hide ();
		}
	}

	if (fade_in_handle == 0) {
		return;
	}

	fade_in_handle->show ();

	/* smallest size for a fade is 64 frames */

	width = std::max ((framecnt_t) 64, width);

	Points* points;

	/* round here to prevent little visual glitches with sub-pixel placement */
	double const pwidth = rint (width / frames_per_pixel);
	uint32_t npoints = std::min (gdk_screen_width(), (int) pwidth);
	double h;

	if (_height < 5) {
		fade_in_shape->hide();
		fade_in_handle->hide();
		return;
	}

	double const handle_center = pwidth;

	/* Put the fade in handle so that its left side is at the end-of-fade line */
	fade_in_handle->set_x0 (handle_center);
	fade_in_handle->set_x1 (handle_center + 6);

	if (pwidth < 5) {
		fade_in_shape->hide();
		return;
	}

	if (trackview.session()->config.get_show_region_fades()) {
		fade_in_shape->show();
	}

	float curve[npoints];
	audio_region()->fade_in()->curve().get_vector (0, audio_region()->fade_in()->back()->when, curve, npoints);

	points = get_canvas_points ("fade in shape", npoints + 3);

	if (_height >= NAME_HIGHLIGHT_THRESH) {
		h = _height - NAME_HIGHLIGHT_SIZE - 2;
	} else {
		h = _height;
	}

	/* points *MUST* be in anti-clockwise order */

	uint32_t pi, pc;
	double xdelta = pwidth/npoints;

	for (pi = 0, pc = 0; pc < npoints; ++pc) {
		(*points)[pi].x = 1 + (pc * xdelta);
		(*points)[pi++].y = 2 + (h - (curve[pc] * h));
	}

	/* fold back */

	(*points)[pi].x = pwidth;
	(*points)[pi++].y = 2;

	(*points)[pi].x = 1;
	(*points)[pi++].y = 2;

	/* connect the dots ... */

	(*points)[pi] = (*points)[0];

	fade_in_shape->set (*points);
	delete points;

	/* ensure trim handle stays on top */
	if (frame_handle_start) {
		frame_handle_start->raise_to_top();
	}
}

void
AudioRegionView::reset_fade_out_shape ()
{
	reset_fade_out_shape_width ((framecnt_t) audio_region()->fade_out()->back()->when);
}

void
AudioRegionView::reset_fade_out_shape_width (framecnt_t width)
{
	if (dragging()) {
		return;
	}

	if (audio_region()->fade_out_is_xfade()) {
		if (fade_out_handle) {
			fade_out_handle->hide ();
			fade_out_shape->hide ();
		}
		redraw_end_xfade ();
		return;
	} else {
		if (end_xfade_in) {
			end_xfade_in->hide ();
			end_xfade_out->hide ();
			end_xfade_rect->hide ();
		}
	}

	if (fade_out_handle == 0) {
		return;
	}

	fade_out_handle->show ();

	/* smallest size for a fade is 64 frames */

	width = std::max ((framecnt_t) 64, width);

	Points* points;

	/* round here to prevent little visual glitches with sub-pixel placement */
	double const pwidth = rint (width / frames_per_pixel);
	uint32_t npoints = std::min (gdk_screen_width(), (int) pwidth);
	double h;

	if (_height < 5) {
		fade_out_shape->hide();
		fade_out_handle->hide();
		return;
	}

	double const handle_center = (_region->length() - width) / frames_per_pixel;

	/* Put the fade out handle so that its right side is at the end-of-fade line;
	 * it's `one out' for precise pixel accuracy.
	 */
	fade_out_handle->set_x0 (handle_center - 5);
	fade_out_handle->set_x1 (handle_center + 1);

	/* don't show shape if its too small */

	if (pwidth < 5) {
		fade_out_shape->hide();
		return;
	}

	if (trackview.session()->config.get_show_region_fades()) {
		fade_out_shape->show();
	}

	float curve[npoints];
	audio_region()->fade_out()->curve().get_vector (0, audio_region()->fade_out()->back()->when, curve, npoints);

	if (_height >= NAME_HIGHLIGHT_THRESH) {
		h = _height - NAME_HIGHLIGHT_SIZE - 2;
	} else {
		h = _height;
	}

	/* points *MUST* be in anti-clockwise order */

	points = get_canvas_points ("fade out shape", npoints + 3);

	uint32_t pi, pc;
	double xdelta = pwidth/npoints;

	for (pi = 0, pc = 0; pc < npoints; ++pc) {
		(*points)[pi].x = _pixel_width - pwidth + (pc * xdelta);
		(*points)[pi++].y = 2 + (h - (curve[pc] * h));
	}

	/* fold back */

	(*points)[pi].x = _pixel_width;
	(*points)[pi++].y = h;

	(*points)[pi].x = _pixel_width;
	(*points)[pi++].y = 2;

	/* connect the dots ... */

	(*points)[pi] = (*points)[0];

	fade_out_shape->set (*points);
	delete points;

	/* ensure trim handle stays on top */
	if (frame_handle_end) {
		frame_handle_end->raise_to_top();
	}
}

void
AudioRegionView::set_frames_per_pixel (double fpp)
{
	RegionView::set_frames_per_pixel (fpp);

	if (_flags & WaveformVisible) {
		for (uint32_t n = 0; n < waves.size(); ++n) {
			waves[n]->set_frames_per_pixel (fpp);
		}
	}

	if (gain_line) {
		gain_line->reset ();
	}

	reset_fade_shapes ();
}

void
AudioRegionView::set_amplitude_above_axis (gdouble spp)
{
	for (uint32_t n=0; n < waves.size(); ++n) {
		waves[n]->property_amplitude_above_axis() = spp;
	}
}

void
AudioRegionView::compute_colors (Gdk::Color const & basic_color)
{
	RegionView::compute_colors (basic_color);

	/* gain color computed in envelope_active_changed() */

	fade_color = UINT_RGBA_CHANGE_A (fill_color, 120);
}

void
AudioRegionView::set_colors ()
{
	RegionView::set_colors();

	if (gain_line) {
		gain_line->set_line_color (audio_region()->envelope_active() ? ARDOUR_UI::config()->canvasvar_GainLine.get() : ARDOUR_UI::config()->canvasvar_GainLineInactive.get());
	}

	for (uint32_t n=0; n < waves.size(); ++n) {
		if (_region->muted()) {
			waves[n]->set_outline_color (UINT_RGBA_CHANGE_A(ARDOUR_UI::config()->canvasvar_WaveForm.get(), MUTED_ALPHA));
		} else {
			waves[n]->set_outline_color (ARDOUR_UI::config()->canvasvar_WaveForm.get());
		}

		waves[n]->property_clip_color() = ARDOUR_UI::config()->canvasvar_WaveFormClip.get();
		waves[n]->property_zero_color() = ARDOUR_UI::config()->canvasvar_ZeroLine.get();
	}
}

void
AudioRegionView::set_waveform_visible (bool yn)
{
	if (((_flags & WaveformVisible) != yn)) {
		if (yn) {
			for (uint32_t n=0; n < waves.size(); ++n) {
				/* make sure the zoom level is correct, since we don't update
				   this when waveforms are hidden.
				*/
				waves[n]->set_frames_per_pixel (frames_per_pixel);
				waves[n]->show();
			}
			_flags |= WaveformVisible;
		} else {
			for (uint32_t n=0; n < waves.size(); ++n) {
				waves[n]->hide();
			}
			_flags &= ~WaveformVisible;
		}
		store_flags ();
	}
}

void
AudioRegionView::temporarily_hide_envelope ()
{
	if (gain_line) {
		gain_line->hide ();
	}
}

void
AudioRegionView::unhide_envelope ()
{
	if (gain_line) {
		gain_line->show ();
	}
}

void
AudioRegionView::set_envelope_visible (bool yn)
{
	if (gain_line) {
		if (yn) {
			gain_line->show ();
		} else {
			gain_line->hide ();
		}
	}
}

void
AudioRegionView::create_waves ()
{
	// cerr << "AudioRegionView::create_waves() called on " << this << endl;//DEBUG
	RouteTimeAxisView& atv (*(dynamic_cast<RouteTimeAxisView*>(&trackview))); // ick

	if (!atv.track()) {
		return;
	}

	ChanCount nchans = atv.track()->n_channels();

	// cerr << "creating waves for " << _region->name() << " with wfd = " << wait_for_data
	//		<< " and channels = " << nchans.n_audio() << endl;

	/* in tmp_waves, set up null pointers for each channel so the vector is allocated */
	for (uint32_t n = 0; n < nchans.n_audio(); ++n) {
		tmp_waves.push_back (0);
	}

	for (vector<ScopedConnection*>::iterator i = _data_ready_connections.begin(); i != _data_ready_connections.end(); ++i) {
		delete *i;
	}

	_data_ready_connections.clear ();

	for (uint32_t i = 0; i < nchans.n_audio(); ++i) {
		_data_ready_connections.push_back (0);
	}

	for (uint32_t n = 0; n < nchans.n_audio(); ++n) {

		if (n >= audio_region()->n_channels()) {
			break;
		}

		// cerr << "\tchannel " << n << endl;

		if (wait_for_data) {
			if (audio_region()->audio_source(n)->peaks_ready (boost::bind (&AudioRegionView::peaks_ready_handler, this, n), &_data_ready_connections[n], gui_context())) {
				// cerr << "\tData is ready\n";
				create_one_wave (n, true);
			} else {
				// cerr << "\tdata is not ready\n";
				// we'll get a PeaksReady signal from the source in the future
				// and will call create_one_wave(n) then.
			}

		} else {
			// cerr << "\tdon't delay, display today!\n";
			create_one_wave (n, true);
		}

	}
}

void
AudioRegionView::create_one_wave (uint32_t which, bool /*direct*/)
{
	//cerr << "AudioRegionView::create_one_wave() called which: " << which << " this: " << this << endl;//DEBUG
	RouteTimeAxisView& atv (*(dynamic_cast<RouteTimeAxisView*>(&trackview))); // ick
	uint32_t nchans = atv.track()->n_channels().n_audio();
	uint32_t n;
	uint32_t nwaves = std::min (nchans, audio_region()->n_channels());
	gdouble ht;

	if (trackview.current_height() < NAME_HIGHLIGHT_THRESH) {
		ht = ((trackview.current_height()) / (double) nchans);
	} else {
		ht = ((trackview.current_height() - NAME_HIGHLIGHT_SIZE) / (double) nchans);
	}

	gdouble yoff = which * ht;

	WaveView *wave = new WaveView (group, audio_region ());

	wave->set_channel (which);
	wave->set_x_position (0);
	wave->set_y_position (yoff);
	wave->set_height (ht);
	wave->set_frames_per_pixel (frames_per_pixel);
	wave->property_amplitude_above_axis() =  _amplitude_above_axis;

	if (_recregion) {
		wave->set_outline_color (_region->muted() ? UINT_RGBA_CHANGE_A(ARDOUR_UI::config()->canvasvar_RecWaveForm.get(), MUTED_ALPHA) : ARDOUR_UI::config()->canvasvar_RecWaveForm.get());
		wave->set_fill_color (ARDOUR_UI::config()->canvasvar_RecWaveFormFill.get());
	} else {
		wave->set_outline_color (_region->muted() ? UINT_RGBA_CHANGE_A(ARDOUR_UI::config()->canvasvar_WaveForm.get(), MUTED_ALPHA) : ARDOUR_UI::config()->canvasvar_WaveForm.get());
		wave->set_fill_color (ARDOUR_UI::config()->canvasvar_WaveFormFill.get());
	}

	wave->property_clip_color() = ARDOUR_UI::config()->canvasvar_WaveFormClip.get();
	wave->property_zero_color() = ARDOUR_UI::config()->canvasvar_ZeroLine.get();
	wave->set_region_start (_region->start());
	wave->property_rectified() = (bool) (_flags & WaveformRectified);
	wave->property_logscaled() = (bool) (_flags & WaveformLogScaled);

	if (!(_flags & WaveformVisible)) {
		wave->hide();
	}

	/* note: calling this function is serialized by the lock
	   held in the peak building thread that signals that
	   peaks are ready for use *or* by the fact that it is
	   called one by one from the GUI thread.
	*/

	if (which < nchans) {
		tmp_waves[which] = wave;
	} else {
		/* n-channel track, >n-channel source */
	}

	/* see if we're all ready */

	for (n = 0; n < nchans; ++n) {
		if (tmp_waves[n] == 0) {
			break;
		}
	}

	if (n == nwaves && waves.empty()) {
		/* all waves are ready */
		tmp_waves.resize(nwaves);

		waves = tmp_waves;
		tmp_waves.clear ();

		/* all waves created, don't hook into peaks ready anymore */
		delete _data_ready_connections[which];
		_data_ready_connections[which] = 0;
	}
}

void
AudioRegionView::peaks_ready_handler (uint32_t which)
{
	Gtkmm2ext::UI::instance()->call_slot (invalidator (*this), boost::bind (&AudioRegionView::create_one_wave, this, which, false));
	// cerr << "AudioRegionView::peaks_ready_handler() called on " << which << " this: " << this << endl;
}

void
AudioRegionView::add_gain_point_event (Canvas::Item *item, GdkEvent *ev)
{
	if (!gain_line) {
		return;
	}

	double x, y;

	/* don't create points that can't be seen */

	set_envelope_visible (true);

	x = ev->button.x;
	y = ev->button.y;

	item->canvas_to_item (x, y);

	framepos_t fx = trackview.editor().pixel_to_frame (x);

	if (fx > _region->length()) {
		return;
	}

	/* compute vertical fractional position */

	y = 1.0 - (y / (_height - NAME_HIGHLIGHT_SIZE));

	/* map using gain line */

	gain_line->view_to_model_coord (x, y);

	/* XXX STATEFUL: can't convert to stateful diff until we
	   can represent automation data with it.
	*/

	trackview.session()->begin_reversible_command (_("add gain control point"));
	XMLNode &before = audio_region()->envelope()->get_state();

	if (!audio_region()->envelope_active()) {
		XMLNode &region_before = audio_region()->get_state();
		audio_region()->set_envelope_active(true);
		XMLNode &region_after = audio_region()->get_state();
		trackview.session()->add_command (new MementoCommand<AudioRegion>(*(audio_region().get()), &region_before, &region_after));
	}

	audio_region()->envelope()->add (fx, y);

	XMLNode &after = audio_region()->envelope()->get_state();
	trackview.session()->add_command (new MementoCommand<AutomationList>(*audio_region()->envelope().get(), &before, &after));
	trackview.session()->commit_reversible_command ();
}

void
AudioRegionView::remove_gain_point_event (Canvas::Item *item, GdkEvent */*ev*/)
{
	ControlPoint *cp = reinterpret_cast<ControlPoint *> (item->get_data ("control_point"));
	audio_region()->envelope()->erase (cp->model());
}

void
AudioRegionView::store_flags()
{
	XMLNode *node = new XMLNode ("GUI");

	node->add_property ("waveform-visible", (_flags & WaveformVisible) ? "yes" : "no");
	node->add_property ("waveform-rectified", (_flags & WaveformRectified) ? "yes" : "no");
	node->add_property ("waveform-logscaled", (_flags & WaveformLogScaled) ? "yes" : "no");

	_region->add_extra_xml (*node);
}

void
AudioRegionView::set_flags (XMLNode* node)
{
	XMLProperty *prop;

	if ((prop = node->property ("waveform-visible")) != 0) {
		if (string_is_affirmative (prop->value())) {
			_flags |= WaveformVisible;
		}
	}

	if ((prop = node->property ("waveform-rectified")) != 0) {
		if (string_is_affirmative (prop->value())) {
			_flags |= WaveformRectified;
		}
	}

	if ((prop = node->property ("waveform-logscaled")) != 0) {
		if (string_is_affirmative (prop->value())) {
			_flags |= WaveformLogScaled;
		}
	}
}

void
AudioRegionView::set_waveform_shape (WaveformShape shape)
{
	bool yn;

	/* this slightly odd approach is to leave the door open to
	   other "shapes" such as spectral displays, etc.
	*/

	switch (shape) {
	case Rectified:
		yn = true;
		break;

	default:
		yn = false;
		break;
	}

	if (yn != (bool) (_flags & WaveformRectified)) {
		for (vector<WaveView *>::iterator wave = waves.begin(); wave != waves.end() ; ++wave) {
			(*wave)->property_rectified() = yn;
		}

		if (yn) {
			_flags |= WaveformRectified;
		} else {
			_flags &= ~WaveformRectified;
		}
		store_flags ();
	}
}

void
AudioRegionView::set_waveform_scale (WaveformScale scale)
{
	bool yn = (scale == Logarithmic);

	if (yn != (bool) (_flags & WaveformLogScaled)) {
		for (vector<WaveView *>::iterator wave = waves.begin(); wave != waves.end() ; ++wave) {
			(*wave)->property_logscaled() = yn;
		}

		if (yn) {
			_flags |= WaveformLogScaled;
		} else {
			_flags &= ~WaveformLogScaled;
		}
		store_flags ();
	}
}


GhostRegion*
AudioRegionView::add_ghost (TimeAxisView& tv)
{
	RouteTimeAxisView* rtv = dynamic_cast<RouteTimeAxisView*>(&trackview);
	assert(rtv);

	double unit_position = _region->position () / frames_per_pixel;
	AudioGhostRegion* ghost = new AudioGhostRegion (tv, trackview, unit_position);
	uint32_t nchans;

	nchans = rtv->track()->n_channels().n_audio();

	for (uint32_t n = 0; n < nchans; ++n) {

		if (n >= audio_region()->n_channels()) {
			break;
		}

		WaveView *wave = new WaveView (ghost->group, audio_region());

		wave->set_channel (n);
		wave->set_x_position (0);
		wave->set_frames_per_pixel (frames_per_pixel);
		wave->property_amplitude_above_axis() =  _amplitude_above_axis;
		wave->set_region_start (_region->start());

		ghost->waves.push_back(wave);
	}

	ghost->set_height ();
	ghost->set_duration (_region->length() / frames_per_pixel);
	ghost->set_colors();
	ghosts.push_back (ghost);

	return ghost;
}

void
AudioRegionView::entered (bool internal_editing)
{
	trackview.editor().set_current_trimmable (_region);
	trackview.editor().set_current_movable (_region);

	if (gain_line && Config->get_show_region_gain ()) {
		gain_line->show_all_control_points ();
	}

	if (fade_in_handle && !internal_editing) {
		fade_in_handle->set_fill_color (UINT_RGBA_CHANGE_A (fade_color, 255));
		fade_out_handle->set_fill_color (UINT_RGBA_CHANGE_A (fade_color, 255));
	}
}

void
AudioRegionView::exited ()
{
	trackview.editor().set_current_trimmable (boost::shared_ptr<Trimmable>());
	trackview.editor().set_current_movable (boost::shared_ptr<Movable>());

	if (gain_line) {
		gain_line->hide_all_but_selected_control_points ();
	}

	if (fade_in_handle) {
		fade_in_handle->set_fill_color (UINT_RGBA_CHANGE_A (fade_color, 0));
		fade_out_handle->set_fill_color (UINT_RGBA_CHANGE_A (fade_color, 0));
	}
}

void
AudioRegionView::envelope_active_changed ()
{
	if (gain_line) {
		gain_line->set_line_color (audio_region()->envelope_active() ? ARDOUR_UI::config()->canvasvar_GainLine.get() : ARDOUR_UI::config()->canvasvar_GainLineInactive.get());
	}
}

void
AudioRegionView::color_handler ()
{
	//case cMutedWaveForm:
	//case cWaveForm:
	//case cWaveFormClip:
	//case cZeroLine:
	set_colors ();

	//case cGainLineInactive:
	//case cGainLine:
	envelope_active_changed();

}

void
AudioRegionView::set_frame_color ()
{
	if (!frame) {
		return;
	}

	if (_region->opaque()) {
		fill_opacity = 130;
	} else {
		fill_opacity = 0;
	}

	TimeAxisViewItem::set_frame_color ();

        uint32_t wc;
        uint32_t fc;

	if (_selected) {
                if (_region->muted()) {
                        wc = UINT_RGBA_CHANGE_A(ARDOUR_UI::config()->canvasvar_SelectedWaveForm.get(), MUTED_ALPHA);
                } else {
                        wc = ARDOUR_UI::config()->canvasvar_SelectedWaveForm.get();
                }
                fc = ARDOUR_UI::config()->canvasvar_SelectedWaveFormFill.get();
	} else {
		if (_recregion) {
                        if (_region->muted()) {
                                wc = UINT_RGBA_CHANGE_A(ARDOUR_UI::config()->canvasvar_RecWaveForm.get(), MUTED_ALPHA);
                        } else {
                                wc = ARDOUR_UI::config()->canvasvar_RecWaveForm.get();
                        }
                        fc = ARDOUR_UI::config()->canvasvar_RecWaveFormFill.get();
		} else {
                        if (_region->muted()) {
                                wc = UINT_RGBA_CHANGE_A(ARDOUR_UI::config()->canvasvar_WaveForm.get(), MUTED_ALPHA);
                        } else {
                                wc = ARDOUR_UI::config()->canvasvar_WaveForm.get();
                        }
                        fc = ARDOUR_UI::config()->canvasvar_WaveFormFill.get();
		}
	}

        for (vector<Canvas::WaveView*>::iterator w = waves.begin(); w != waves.end(); ++w) {
		(*w)->set_outline_color (wc);
                if (!_region->muted()) {
                        (*w)->set_fill_color (fc);
                }
        }
}

void
AudioRegionView::set_fade_visibility (bool yn)
{
	if (yn) {
		if (fade_in_shape) {
			fade_in_shape->show();
		}
		if (fade_out_shape) {
			fade_out_shape->show ();
		}
		if (fade_in_handle) {
			fade_in_handle->show ();
		}
		if (fade_out_handle) {
			fade_out_handle->show ();
		}
	} else {
		if (fade_in_shape) {
			fade_in_shape->hide();
		}
		if (fade_out_shape) {
			fade_out_shape->hide ();
		}
		if (fade_in_handle) {
			fade_in_handle->hide ();
		}
		if (fade_out_handle) {
			fade_out_handle->hide ();
		}
	}
}

void
AudioRegionView::update_coverage_frames (LayerDisplay d)
{
	RegionView::update_coverage_frames (d);

	if (fade_in_handle) {
		fade_in_handle->raise_to_top ();
		fade_out_handle->raise_to_top ();
	}
}

void
AudioRegionView::show_region_editor ()
{
	if (editor == 0) {
		editor = new AudioRegionEditor (trackview.session(), audio_region());
	}

	editor->present ();
	editor->set_position (Gtk::WIN_POS_MOUSE);
	editor->show_all();
}


void
AudioRegionView::show_fade_line (framepos_t pos)
{
	fade_position_line->set_x0 (trackview.editor().frame_to_pixel (pos));
	fade_position_line->set_x1 (trackview.editor().frame_to_pixel (pos));
	fade_position_line->show ();
	fade_position_line->raise_to_top ();
}

void
AudioRegionView::hide_fade_line ()
{
	fade_position_line->hide ();
}


void
AudioRegionView::transients_changed ()
{
	AnalysisFeatureList analysis_features = _region->transients();

	while (feature_lines.size() < analysis_features.size()) {

		Canvas::Line* canvas_item = new Canvas::Line (group);
	
		canvas_item->set (
			Canvas::Duple (-1.0, 2.0),
			Canvas::Duple (1.0, _height - TimeAxisViewItem::NAME_HIGHLIGHT_SIZE - 1)
			);

		canvas_item->set_outline_width (1);
		canvas_item->property_first_arrowhead() = TRUE;
		canvas_item->property_last_arrowhead() = TRUE;
		canvas_item->property_arrow_shape_a() = 11.0;
		canvas_item->property_arrow_shape_b() = 0.0;
		canvas_item->property_arrow_shape_c() = 4.0;

		canvas_item->raise_to_top ();
		canvas_item->show ();

		canvas_item->set_data ("regionview", this);
		canvas_item->Event.connect (sigc::bind (sigc::mem_fun (PublicEditor::instance(), &PublicEditor::canvas_feature_line_event), canvas_item, this));
		
		feature_lines.push_back (make_pair(0, canvas_item));		
	}

	while (feature_lines.size() > analysis_features.size()) {
		Canvas::Line* line = feature_lines.back().second;
		feature_lines.pop_back ();
		delete line;
	}

	AnalysisFeatureList::const_iterator i;
	list<std::pair<framepos_t, Canvas::Line*> >::iterator l;
	for (i = analysis_features.begin(), l = feature_lines.begin(); i != analysis_features.end() && l != feature_lines.end(); ++i, ++l) {

		float *pos = new float;
		*pos = trackview.editor().frame_to_pixel (*i);

		(*l).second->set (
			Canvas::Duple (*pos, 2.0),
			Canvas::Duple (*pos, _height - TimeAxisViewItem::NAME_HIGHLIGHT_SIZE - 1)
			);
		
		(*l).second->set_data ("position", pos);

		(*l).first = *i;
	}
}

void
AudioRegionView::update_transient(float /*old_pos*/, float new_pos)
{
	/* Find frame at old pos, calulate new frame then update region transients*/
	list<std::pair<framepos_t, Canvas::Line*> >::iterator l;

	for (l = feature_lines.begin(); l != feature_lines.end(); ++l) {

		/* Line has been updated in drag so we compare to new_pos */

		float* pos = (float*) (*l).second->get_data ("position");

		if (rint(new_pos) == rint(*pos)) {

		    framepos_t old_frame = (*l).first;
		    framepos_t new_frame = trackview.editor().pixel_to_frame (new_pos);

		    _region->update_transient (old_frame, new_frame);

		    break;
		}
	}
}

void
AudioRegionView::remove_transient(float pos)
{
	/* Find frame at old pos, calulate new frame then update region transients*/
	list<std::pair<framepos_t, Canvas::Line*> >::iterator l;

	for (l = feature_lines.begin(); l != feature_lines.end(); ++l) {

		/* Line has been updated in drag so we compare to new_pos */
		float *line_pos = (float*) (*l).second->get_data ("position");

		if (rint(pos) == rint(*line_pos)) {
		    _region->remove_transient ((*l).first);
		    break;
		}
	}
}

void
AudioRegionView::thaw_after_trim ()
{
	RegionView::thaw_after_trim ();
	unhide_envelope ();
	drag_end ();
}

void
AudioRegionView::redraw_start_xfade ()
{
	boost::shared_ptr<AudioRegion> ar (audio_region());

	if (!ar->fade_in() || ar->fade_in()->empty()) {
		return;
	}

	if (!ar->fade_in_is_xfade()) {
		if (start_xfade_in) {
			start_xfade_in->hide ();
			start_xfade_out->hide ();
			start_xfade_rect->hide ();
		}
		return;
	}

	redraw_start_xfade_to (ar, ar->fade_in()->back()->when);
}

void
AudioRegionView::redraw_start_xfade_to (boost::shared_ptr<AudioRegion> ar, framecnt_t len)
{
	int32_t const npoints = trackview.editor().frame_to_pixel (len);

	if (npoints < 3) {
		return;
	}

	if (!start_xfade_in) {
		start_xfade_in = new Canvas::PolyLine (group);
		start_xfade_in->set_outline_width (1);
		start_xfade_in->set_outline_color (ARDOUR_UI::config()->canvasvar_GainLine.get());
	}

	if (!start_xfade_out) {
		start_xfade_out = new Canvas::PolyLine (group);
		start_xfade_out->set_outline_width (1);
		uint32_t col = UINT_RGBA_CHANGE_A (ARDOUR_UI::config()->canvasvar_GainLine.get(), 125);
		start_xfade_out->set_outline_color (col);
	}

	if (!start_xfade_rect) {
		start_xfade_rect = new Canvas::Rectangle (group);
		start_xfade_rect->set_outline (true);
		start_xfade_rect->set_fill (true);
		start_xfade_rect->set_fill_color (ARDOUR_UI::config()->canvasvar_ActiveCrossfade.get());
		start_xfade_rect->set_outline_width (0);
		start_xfade_rect->Event.connect (sigc::bind (sigc::mem_fun (PublicEditor::instance(), &PublicEditor::canvas_start_xfade_event), start_xfade_rect, this));
		start_xfade_rect->set_data ("regionview", this);
	}

	Points* points = get_canvas_points ("xfade edit redraw", npoints);
	boost::scoped_ptr<float> vec (new float[npoints]);
	double effective_height = _height - NAME_HIGHLIGHT_SIZE - 1.0;

	ar->fade_in()->curve().get_vector (0, ar->fade_in()->back()->when, vec.get(), npoints);

	for (int i = 0, pci = 0; i < npoints; ++i) {
		Canvas::Duple &p ((*points)[pci++]);
		p.x = i;
		p.y = 1.0 + effective_height - (effective_height * vec.get()[i]);
	}

	start_xfade_rect->set_x0 (((*points)[0]).x);
	start_xfade_rect->set_y0 (1);
	start_xfade_rect->set_x1 (((*points)[npoints-1]).x);
	start_xfade_rect->set_y1 (effective_height);
	start_xfade_rect->show ();
	start_xfade_rect->raise_to_top ();

	start_xfade_in->set (*points);
	start_xfade_in->show ();
	start_xfade_in->raise_to_top ();

	/* fade out line */

	boost::shared_ptr<AutomationList> inverse = ar->inverse_fade_in();

	if (!inverse) {

		for (int i = 0, pci = 0; i < npoints; ++i) {
			Canvas::Duple &p ((*points)[pci++]);
			p.x = i;
			p.y = 1.0 + effective_height - (effective_height * (1.0 - vec.get()[i]));
		}

	} else {

		inverse->curve().get_vector (0, inverse->back()->when, vec.get(), npoints);

		for (int i = 0, pci = 0; i < npoints; ++i) {
			Canvas::Duple &p ((*points)[pci++]);
			p.x = i;
			p.y = 1.0 + effective_height - (effective_height * vec.get()[i]);
		}
	}

	start_xfade_out->set (*points);
	start_xfade_out->show ();
	start_xfade_out->raise_to_top ();

	delete points;
}

void
AudioRegionView::redraw_end_xfade ()
{
	boost::shared_ptr<AudioRegion> ar (audio_region());

	if (!ar->fade_out() || ar->fade_out()->empty()) {
		return;
	}

	if (!ar->fade_out_is_xfade()) {
		if (end_xfade_in) {
			end_xfade_in->hide ();
			end_xfade_out->hide ();
			end_xfade_rect->hide ();
		}
		return;
	}

	redraw_end_xfade_to (ar, ar->fade_out()->back()->when);
}

void
AudioRegionView::redraw_end_xfade_to (boost::shared_ptr<AudioRegion> ar, framecnt_t len)
{
	int32_t const npoints = trackview.editor().frame_to_pixel (len);

	if (npoints < 3) {
		return;
	}

	if (!end_xfade_in) {
		end_xfade_in = new Canvas::PolyLine (group);
		end_xfade_in->set_outline_width (1);
		end_xfade_in->set_outline_color (ARDOUR_UI::config()->canvasvar_GainLine.get());
	}

	if (!end_xfade_out) {
		end_xfade_out = new Canvas::PolyLine (group);
		end_xfade_out->set_outline_width (1);
		uint32_t col UINT_RGBA_CHANGE_A (ARDOUR_UI::config()->canvasvar_GainLine.get(), 125);
		end_xfade_out->set_outline_color (col);
	}

	if (!end_xfade_rect) {
		end_xfade_rect = new Canvas::Rectangle (group);
		end_xfade_rect->set_outline (true);
		end_xfade_rect->set_fill (true);
		end_xfade_rect->set_fill_color (ARDOUR_UI::config()->canvasvar_ActiveCrossfade.get());
		end_xfade_rect->set_outline_width (0);
		end_xfade_rect->Event.connect (sigc::bind (sigc::mem_fun (PublicEditor::instance(), &PublicEditor::canvas_end_xfade_event), end_xfade_rect, this));
		end_xfade_rect->set_data ("regionview", this);
	}

	Points* points = get_canvas_points ("xfade edit redraw", npoints);
	boost::scoped_ptr<float> vec (new float[npoints]);

	ar->fade_out()->curve().get_vector (0, ar->fade_out()->back()->when, vec.get(), npoints);

	double rend = trackview.editor().frame_to_pixel (_region->length() - len);
	double effective_height = _height - NAME_HIGHLIGHT_SIZE - 1;

	for (int i = 0, pci = 0; i < npoints; ++i) {
		Canvas::Duple &p ((*points)[pci++]);
		p.x = rend + i;
		p.y = 1.0 + effective_height - (effective_height * vec.get()[i]);
	}

	end_xfade_rect->set_x0 (((*points)[0]).x);
	end_xfade_rect->set_y0 (1);
	end_xfade_rect->set_x1 (((*points)[npoints-1]).x);
	end_xfade_rect->set_y1 (effective_height);
	end_xfade_rect->show ();
	end_xfade_rect->raise_to_top ();

	end_xfade_in->set (*points);
	end_xfade_in->show ();
	end_xfade_in->raise_to_top ();

	/* fade in line */

	boost::shared_ptr<AutomationList> inverse = ar->inverse_fade_out ();

	if (!inverse) {

		for (int i = 0, pci = 0; i < npoints; ++i) {
			Canvas::Duple &p ((*points)[pci++]);
			p.x = rend + i;
			p.y = 1.0 + effective_height - (effective_height * (1.0 - vec.get()[i]));
		}

	} else {

		inverse->curve().get_vector (inverse->front()->when, inverse->back()->when, vec.get(), npoints);

		for (int i = 0, pci = 0; i < npoints; ++i) {
			Canvas::Duple &p ((*points)[pci++]);
			p.x = rend + i;
			p.y = 1.0 + effective_height - (effective_height * vec.get()[i]);
		}
	}

	end_xfade_out->set (*points);
	end_xfade_out->show ();
	end_xfade_out->raise_to_top ();

	delete points;
}

void
AudioRegionView::hide_xfades ()
{
	if (start_xfade_in) {
		start_xfade_in->hide();
	}
	if (start_xfade_out) {
		start_xfade_out->hide();
	}
	if (start_xfade_rect) {
		start_xfade_rect->hide ();
	}
	if (end_xfade_in) {
		end_xfade_in->hide();
	}
	if (end_xfade_out) {
		end_xfade_out->hide();
	}
	if (end_xfade_rect) {
		end_xfade_rect->hide ();
	}
}

void
AudioRegionView::show_xfades ()
{
	if (start_xfade_in) {
		start_xfade_in->show();
	}
	if (start_xfade_out) {
		start_xfade_out->show();
	}
	if (start_xfade_rect) {
		start_xfade_rect->show ();
	}
	if (end_xfade_in) {
		end_xfade_in->show();
	}
	if (end_xfade_out) {
		end_xfade_out->show();
	}
	if (end_xfade_rect) {
		end_xfade_rect->show ();
	}
}

void
AudioRegionView::drag_start ()
{
	TimeAxisViewItem::drag_start ();
	AudioTimeAxisView* atav = dynamic_cast<AudioTimeAxisView*> (&trackview);

	if (atav) {
		AudioStreamView* av = atav->audio_view();
		if (av) {
			/* this will hide our xfades too */
			av->hide_xfades_with (audio_region());
		}
	}
}

void
AudioRegionView::drag_end ()
{
	TimeAxisViewItem::drag_end ();
	/* fades will be redrawn if they changed */
}

