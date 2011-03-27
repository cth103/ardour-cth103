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

}

#endif

#endif
