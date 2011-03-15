#include <algorithm>
#include "canvas/poly_item.h"

using namespace std;
using namespace ArdourCanvas;

PolyItem::PolyItem (Group* parent)
	: Item (parent)
{

}

void
PolyItem::compute_bounding_box () const
{
	bool have_first = false;

	Rect bbox;

	for (Points::const_iterator i = _points.begin(); i != _points.end(); ++i) {
		if (have_first) {
			bbox.x0 = min (bbox.x0, i->x);
			bbox.y0 = min (bbox.y0, i->y);
			bbox.x1 = max (bbox.x1, i->x);
			bbox.y1 = max (bbox.y1, i->y);
		} else {
			bbox.x0 = bbox.x1 = i->x;
			bbox.y0 = bbox.y1 = i->y;
			have_first = true;
		}
	}

	_bounding_box = bbox;
	_bounding_box_dirty = false;
}

void
PolyItem::render_path (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	bool done_first = false;
	for (Points::const_iterator i = _points.begin(); i != _points.end(); ++i) {
		if (done_first) {
			context->line_to (i->x, i->y);
		} else {
			context->move_to (i->x, i->y);
			done_first = true;
		}
	}
}

void
PolyItem::set (Points const & points)
{
	_points = points;
}

Points const &
PolyItem::get () const
{
	return _points;
}
