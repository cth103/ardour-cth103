#ifndef __CANVAS_UNIMPLEMENTED_H__
#define __CANVAS_UNIMPLEMENTED_H__

#ifdef CANVAS_COMPATIBILITY

#include <gdk/gdkevents.h>
#include <gtkmm/widget.h>
#include "evoral/Note.hpp"
#include "ardour/midi_model.h"
#include "canvas/item.h"

class MidiRegionView;

class GnomeCanvasWaveViewCache {

};

extern void gnome_canvas_waveview_cache_destroy (GnomeCanvasWaveViewCache *);

namespace ArdourCanvas {

class Polygon : public Item {
public:
	Polygon (Group *);

	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;

	void set (Points const &);
	
	Points& property_points ();
	uint32_t& property_fill_color_rgba ();
	double& property_width_pixels ();
	uint32_t& property_outline_color_rgba ();
};

class WaveView : public Item {
public:
	WaveView (Group *);

	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	
	
	static GnomeCanvasWaveViewCache* create_cache ();
	
	void*& property_data_src ();
	void*& property_gain_src ();
	void*& property_cache ();
	bool& property_cache_updater ();
	int& property_channel ();
	void*& property_length_function ();
	void*& property_sourcefile_length_function ();
	void*& property_peak_function ();
	void*& property_gain_function ();
	double& property_x ();
	bool& property_rectified ();
	bool& property_logscaled ();
	ARDOUR::framepos_t& property_region_start ();
	double& property_y ();
	double& property_height ();
	uint32_t& property_wave_color ();
	double& property_samples_per_unit ();
	double& property_amplitude_above_axis ();
	uint32_t& property_clip_color ();
	uint32_t& property_zero_color ();
	uint32_t& property_fill_color ();
};

class Pixbuf : public Item {
public:
	Pixbuf (Group *);
	Pixbuf (Glib::RefPtr<Gdk::Pixbuf>);
	Pixbuf (Group *, double, double, Glib::RefPtr<Gdk::Pixbuf>);

	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	
	
	double& property_x ();
	double& property_y ();
	Glib::RefPtr<Gdk::Pixbuf>& property_pixbuf ();
};

class CanvasPatchChange: public Item {
public:
	CanvasPatchChange (
		MidiRegionView& region,
		Group&          parent,
		const string&   text,
		double          height,
		double          x,
		double          y,
		string&         model_name,
		string&         custom_device_mode,
		ARDOUR::MidiModel::PatchChangePtr patch
		);

	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	
	
	ARDOUR::MidiModel::PatchChangePtr patch () const;
	void set_height (double);
};

class CanvasNoteEvent : public Item {
public:
	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;

	int& property_outline_what ();
	uint32_t& property_outline_color_rgba ();
	
	CanvasNoteEvent (MidiRegionView&, Item*, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> >);
	const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > note ();
	void set_selected (bool);
	bool selected () const;
	double mouse_x_fraction ();
	bool mouse_near_ends ();
	MidiRegionView& region_view ();
	double x1 ();
	double x2 ();
	double y1 ();
	double y2 ();
	void invalidate ();
	void validate ();
	bool valid ();
	void hide_velocity ();
	void show_velocity ();
	void move_event (double, double);
	void on_channel_selection_change (uint16_t);
	static uint32_t meter_style_fill_color (uint8_t, bool);
	static uint32_t calculate_outline (uint32_t);
	static const uint32_t midi_channel_colors[16];

        static PBD::Signal1<void, CanvasNoteEvent*> CanvasNoteEventDeleted;
};

class CanvasNote : public CanvasNoteEvent {
public:
	CanvasNote (MidiRegionView&, Group&, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> >);
	bool big_enough_to_trim ();
	double& property_x1 ();
	double& property_x2 ();
	double& property_y1 ();
	double& property_y2 ();
};

class NoEventCanvasNote : public CanvasNote {
public:
	NoEventCanvasNote (MidiRegionView&, Group&, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> >);

};

class NoEventText : public Item {
public:
	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	
	uint32_t& property_color_rgba ();
	NoEventText (Group *);
	std::string& property_text ();
	double& property_x ();
	double& property_y ();
	Pango::FontDescription& property_font_desc ();
	int& property_anchor ();
	bool mouse_near_ends ();
};

class CanvasSysEx : public Item {
public:
	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	
	
	CanvasSysEx (MidiRegionView &, Group *, std::string, double, double, double);

};

class CanvasHit : public CanvasNoteEvent {
public:
	CanvasHit (MidiRegionView&, Group&, double, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> >);
	void set_height (double);
	const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > note ();
	void move_to (double, double);
};

class LineSet : public Item {
public:
	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	
	
	enum Orientation {
		Vertical,
		Horizontal
	};

	LineSet (Group *, Orientation);

	double& property_x1 ();
	double& property_x2 ();
	double& property_y1 ();
	double& property_y2 ();

	void move_line(double coord, double dest);
	void change_line_width(double coord, double width);
	void change_line_color(double coord, uint32_t color);
	void add_line(double coord, double width, uint32_t color);
	void remove_line(double coord);
	void remove_lines(double c1, double c2);
	void remove_until(double coord);
	void remove_from(double coord);
	void clear();
	void request_lines(double c1, double c2);
	sigc::signal<void, LineSet&, double, double> signal_request_lines;
};
	
}

#endif

#endif
