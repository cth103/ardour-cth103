#include "evoral/Note.hpp"
#include "ardour/midi_model.h"
#include "canvas/item.h"

class MidiRegionView;

namespace ArdourCanvas {

class NoteBase : public Item
{
public:
	void compute_bounding_box () const { _bounding_box_dirty = false; }
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const {}
	XMLNode* get_state () const {
		return new XMLNode ("NoteBase");
	}
	void set_state (XMLNode const *) {}

	void set_outline_what (int) {}
	
	int& property_outline_what () {
		return _foo_int;
	}
	uint32_t& property_outline_color_rgba () {
		return _foo_uint;
	}
	
	NoteBase (MidiRegionView& rv, Item*, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> >) : Item ((Group *) 0), _foo_region_view (rv) {}
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

        static PBD::Signal1<void, NoteBase*> NoteBaseDeleted;

private:
	MidiRegionView& _foo_region_view;
	int _foo_int;
	uint32_t _foo_uint;
};

}
