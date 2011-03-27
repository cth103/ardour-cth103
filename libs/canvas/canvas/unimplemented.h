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
