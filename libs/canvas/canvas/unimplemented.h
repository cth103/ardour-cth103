#ifndef __CANVAS_UNIMPLEMENTED_H__
#define __CANVAS_UNIMPLEMENTED_H__

#ifdef CANVAS_COMPATIBILITY

#include <gdk/gdkevents.h>
#include <gtkmm/widget.h>
#include "pbd/xml++.h"
#include "evoral/Note.hpp"
#include "ardour/midi_model.h"
#include "canvas/item.h"

class MidiRegionView;

namespace ArdourCanvas {

class CanvasPatchChange: public Item {
public:
	CanvasPatchChange (
		MidiRegionView& region,
		Group*          parent,
		const string&   text,
		double          height,
		double          x,
		double          y,
		string&         model_name,
		string&         custom_device_mode,
		ARDOUR::MidiModel::PatchChangePtr patch
		) : Item (parent)
	{}

	void compute_bounding_box () const { _bounding_box_dirty = false; }
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const {}
	XMLNode* get_state () const {
		return new XMLNode ("CanvasPatchChange");
	}
	void set_state (XMLNode const *) {}
	
	ARDOUR::MidiModel::PatchChangePtr patch () const {
		return ARDOUR::MidiModel::PatchChangePtr ();
	}
	
	void set_height (double) {}
};

class CanvasNoteEvent : public Item {
public:
	void compute_bounding_box () const { _bounding_box_dirty = false; }
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const {}
	XMLNode* get_state () const {
		return new XMLNode ("CanvasNoteEvent");
	}
	void set_state (XMLNode const *) {}

	void set_outline_what (int) {}
	
	int& property_outline_what () {
		return _foo_int;
	}
	uint32_t& property_outline_color_rgba () {
		return _foo_uint;
	}
	
	CanvasNoteEvent (MidiRegionView& rv, Item*, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> >) : Item ((Group *) 0), _foo_region_view (rv) {}
	const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > note () {
		return boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > ();
	}
	void set_selected (bool) {}
	bool selected () const {
		return false;
	}
	double mouse_x_fraction () {
		return 0;
	}
	bool mouse_near_ends () {
		return false;
	}
	MidiRegionView& region_view () {
		return _foo_region_view;
	}
	double x1 () {
		return 0;
	}
	double x2 () {
		return 0;
	}
	double y1 () {
		return 0;
	}
	double y2 () {
		return 0;
	}
	void invalidate () {}
	void validate () {}
	bool valid () { return false; }
	void hide_velocity () {}
	void show_velocity () {}
	void move_event (double, double) {}
	void on_channel_selection_change (uint16_t) {}
	
	static uint32_t meter_style_fill_color (uint8_t, bool) {
		return 0;
	}
	static uint32_t calculate_outline (uint32_t) {
		return 0;
	}
	static const uint32_t midi_channel_colors[16];

        static PBD::Signal1<void, CanvasNoteEvent*> CanvasNoteEventDeleted;

private:
	MidiRegionView& _foo_region_view;
	int _foo_int;
	uint32_t _foo_uint;
};

class CanvasNote : public CanvasNoteEvent {
public:
	CanvasNote (MidiRegionView& rv, Group* g, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > n)
		: CanvasNoteEvent (rv, this, n) {}

	bool big_enough_to_trim () {
		return false;
	}
	double& property_x1 () {
		return _foo_double;
	}
	double& property_x2 () {
		return _foo_double;
	}
	double& property_y1 () {
		return _foo_double;
	}
	double& property_y2 () {
		return _foo_double;
	}

private:
	double _foo_double;
};

class NoEventCanvasNote : public CanvasNote {
public:
	NoEventCanvasNote (MidiRegionView& rv, Group* g, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > n)
		: CanvasNote (rv, g, n) {}

};

class NoEventText : public Item {
public:
	NoEventText (Group* p) : Item (p) {}
	
	void compute_bounding_box () const { _bounding_box_dirty = false; }
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const {}
	XMLNode* get_state () const {
		return new XMLNode ("NoEventText");
	}
	void set_state (XMLNode const *) {}

	void set_color (uint32_t) {}
	
	uint32_t& property_color_rgba () {
		return _foo_uint;
	}
	std::string& property_text () {
		return _foo_string;
	}
	double& property_x () {
		return _foo_double;
	}
	double& property_y () {
		return _foo_double;
	}
	Pango::FontDescription& property_font_desc () {
		return _foo_font;
	}
	int& property_anchor () {
		return _foo_int;
	}
	bool mouse_near_ends () {
		return false;
	}

private:
	int _foo_int;
	uint32_t _foo_uint;
	double _foo_double;
	Pango::FontDescription _foo_font;
	std::string _foo_string;
};

class CanvasSysEx : public Item {
public:
	void compute_bounding_box () const { _bounding_box_dirty = false; }
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const {}
	XMLNode* get_state () const {
		return new XMLNode ("CanvasSysEx");
	}
	void set_state (XMLNode const *) {}
	
	CanvasSysEx (MidiRegionView &, Group *, std::string, double, double, double) : Item ((Group *) 0) {}
};

class CanvasHit : public CanvasNoteEvent {
public:
	CanvasHit (MidiRegionView& rv, Group&, double, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > n) 
		: CanvasNoteEvent (rv, this, n) {}
	
	void set_height (double) {}
	const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > note () {
		return boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > ();
	}
	void move_to (double, double) {}
};

class LineSet : public Item {
public:
	void compute_bounding_box () const { _bounding_box_dirty = false; }
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const {}
	XMLNode* get_state () const {
		return new XMLNode ("LineSet");
	}
	void set_state (XMLNode const *) {}
	
	enum Orientation {
		Vertical,
		Horizontal
	};

	LineSet (Group* p, Orientation) : Item (p) {}

	double& property_x1 () {
		return _foo_double;
	}
	double& property_x2 () {
		return _foo_double;
	}
	double& property_y1 () {
		return _foo_double;
	}
	double& property_y2 () {
		return _foo_double;
	}

	void move_line(double coord, double dest) {}
	void change_line_width(double coord, double width) {}
	void change_line_color(double coord, uint32_t color) {}
	void add_line(double coord, double width, uint32_t color) {}
	void remove_line(double coord) {}
	void remove_lines(double c1, double c2) {}
	void remove_until(double coord) {}
	void remove_from(double coord) {}
	void clear() {}
	void request_lines(double c1, double c2) {}
	sigc::signal<void, LineSet&, double, double> signal_request_lines;

private:
	double _foo_double;
};
	
}

#endif

#endif
