#include "canvas/item.h"
#include "canvas/outline.h"

namespace ARDOUR {
	class Session;
}

namespace ArdourCanvas {
	
class BBTLines : virtual public Item, public Outline
{
public:
	BBTLines (Group *, Color, Color);

	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	void compute_bbox () const;
	XMLNode* get_state () const;
	void set_state (XMLNode const *);

	void set_session (ARDOUR::Session *);

	void set_frames_per_pixel (double);
	void tempo_map_changed ();

private:
	ARDOUR::Session* _session;
	double _frames_per_pixel;
	Color _bar_color;
	Color _beat_color;
};

}
