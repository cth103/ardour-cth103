#include <algorithm>
#include "canvas/poly_item.h"

using namespace std;
using namespace ArdourCanvas;

PolyItem::PolyItem (Group* parent)
	: Item (parent)
{

}

boost::optional<Rect>
PolyItem::bounding_box () const
{
	Rect r;
	bool have_first = false;

	for (Points::const_iterator i = _points.begin(); i != _points.end(); ++i) {
		if (have_first) {
			r.x0 = min (r.x0, i->x);
			r.y0 = min (r.y0, i->y);
			r.x1 = max (r.x1, i->x);
			r.y1 = max (r.y1, i->y);
		} else {
			r.x0 = r.x1 = i->x;
			r.y0 = r.y1 = i->y;
			have_first = true;
		}
	}

	return boost::optional<Rect> (r);
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
