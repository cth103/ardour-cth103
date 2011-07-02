/*
    Copyright (C) 2001-2006 Paul Davis

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

#ifndef __gtk_ardour_audio_region_view_h__
#define __gtk_ardour_audio_region_view_h__

#include <vector>

#include <sigc++/signal.h>
#include "ardour/audioregion.h"
#include "canvas/wave_view.h"
#include "region_view.h"
#include "time_axis_view_item.h"
#include "automation_line.h"
#include "enums.h"

namespace ARDOUR {
	class AudioRegion;
	class PeakData;
};

namespace Canvas {
	class WaveView;
	class Line;
	class Rectangle;
	class Polygon;
}

class AudioTimeAxisView;
class AudioRegionGainLine;
class GhostRegion;
class AutomationTimeAxisView;
class RouteTimeAxisView;

class AudioRegionView : public RegionView
{
  public:
	AudioRegionView (Canvas::Group *,
			 RouteTimeAxisView&,
			 boost::shared_ptr<ARDOUR::AudioRegion>,
			 double initial_frames_per_pixel,
			 Gdk::Color const & basic_color);

	AudioRegionView (Canvas::Group *,
			 RouteTimeAxisView&,
			 boost::shared_ptr<ARDOUR::AudioRegion>,
			 double frames_per_pixel,
			 Gdk::Color const & basic_color,
			 bool recording,
			 TimeAxisViewItem::Visibility);

	AudioRegionView (const AudioRegionView& other, boost::shared_ptr<ARDOUR::AudioRegion>);

	~AudioRegionView ();

	virtual void init (Gdk::Color const & base_color, bool wait_for_data);

	boost::shared_ptr<ARDOUR::AudioRegion> audio_region() const;

	void create_waves ();

	void set_height (double);
	void set_frames_per_pixel (double);

	void set_amplitude_above_axis (gdouble spp);

	void temporarily_hide_envelope (); ///< Dangerous!
	void unhide_envelope ();           ///< Dangerous!

	void set_envelope_visible (bool);
	void set_waveform_visible (bool yn);
	void set_waveform_shape (ARDOUR::WaveformShape);
	void set_waveform_scale (ARDOUR::WaveformScale);

	bool waveform_rectified() const { return _flags & WaveformRectified; }
	bool waveform_logscaled() const { return _flags & WaveformLogScaled; }
	bool waveform_visible()   const { return _flags & WaveformVisible; }
	bool envelope_visible()   const { return _flags & EnvelopeVisible; }

	void add_gain_point_event (Canvas::Item *item, GdkEvent *event);
	void remove_gain_point_event (Canvas::Item *item, GdkEvent *event);

	AudioRegionGainLine* get_gain_line() const { return gain_line; }

	void region_changed (const PBD::PropertyChange&);
	void envelope_active_changed ();

	GhostRegion* add_ghost (TimeAxisView&);

	void reset_fade_in_shape_width (framecnt_t);
	void reset_fade_out_shape_width (framecnt_t);

	void show_fade_line(framepos_t pos);
	void hide_fade_line();

	void set_fade_visibility (bool);
	void update_coverage_frames (LayerDisplay);

	void update_transient(float old_pos, float new_pos);
	void remove_transient(float pos);

	void show_region_editor ();

	virtual void entered (bool);
	virtual void exited ();

	void thaw_after_trim ();

  protected:

	/* this constructor allows derived types
	   to specify their visibility requirements
	   to the TimeAxisViewItem parent class
	*/

	enum Flags {
		EnvelopeVisible = 0x1,
		WaveformVisible = 0x4,
		WaveformRectified = 0x8,
		WaveformLogScaled = 0x10,
	};
	
	std::vector<Canvas::WaveView *> waves;
	std::vector<Canvas::WaveView *> tmp_waves; ///< see ::create_waves()

	std::list<std::pair<framepos_t, Canvas::Line*> > feature_lines;
	
	Canvas::Polygon*           sync_mark; ///< polgyon for sync position
	Canvas::Polygon*           fade_in_shape;
	Canvas::Polygon*           fade_out_shape;
	Canvas::Rectangle*         fade_in_handle; ///< fade in handle, or 0
	Canvas::Rectangle*         fade_out_handle; ///< fade out handle, or 0
	Canvas::Line*              fade_position_line;
	
	AudioRegionGainLine * gain_line;

	double _amplitude_above_axis;

	uint32_t _flags;
	uint32_t fade_color;

	void reset_fade_shapes ();
	void reset_fade_in_shape ();
	void reset_fade_out_shape ();
	void fade_in_changed ();
	void fade_out_changed ();
	void fade_in_active_changed ();
	void fade_out_active_changed ();

	void region_resized (const PBD::PropertyChange&);
	void region_muted ();
	void region_scale_amplitude_changed ();
	void region_renamed ();

	void create_one_wave (uint32_t, bool);
	void peaks_ready_handler (uint32_t);
	void set_flags (XMLNode *);
	void store_flags ();

	void set_colors ();
	void compute_colors (Gdk::Color const &);
	void reset_width_dependent_items (double pixel_width);
	void set_frame_color ();

	void color_handler ();
<<<<<<< HEAD
	
=======

	std::vector<GnomeCanvasWaveViewCache*> wave_caches;

>>>>>>> origin/master
	void transients_changed();

private:

	void setup_fade_handle_positions ();

	/** A ScopedConnection for each PeaksReady callback (one per channel).  Each member
	 *  may be 0 if no connection exists.
	 */
	std::vector<PBD::ScopedConnection*> _data_ready_connections;
};

#endif /* __gtk_ardour_audio_region_view_h__ */
