#include "canvas/note_base.h"

namespace ArdourCanvas {

class Note : public NoteBase
{
public:
	Note (MidiRegionView& rv, Group* g, const boost::shared_ptr<Evoral::Note<ARDOUR::MidiModel::TimeType> > n)
		: NoteBase (rv, this, n) {}

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

}
