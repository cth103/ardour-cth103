#include <algorithm>
#include <cairomm/context.h>
#include "canvas/line.h"
#include "canvas/types.h"
#include "canvas/debug.h"
#include "canvas/utils.h"

using namespace std;
using namespace ArdourCanvas;

Line::Line (Group* parent)
	: Item (parent)
{

}

boost::optional<Rect>
Line::bounding_box () const
{
	Rect r;
	r.x0 = min (_points[0].x, _points[1].x);
	r.y0 = min (_points[0].y, _points[1].y);
	r.x1 = max (_points[0].x, _points[1].x);
	r.y1 = max (_points[0].y, _points[1].y);
	return boost::optional<Rect> (r);
}

void
Line::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	setup_context (context);

	context->move_to (_points[0].x, _points[0].y);
	context->line_to (_points[1].x, _points[1].y);
	context->stroke ();

	Debug::instance()->render_object_count++;
}

void
Line::set (Point a, Point b)
{
	_points[0] = a;
	_points[1] = b;
}

void
Line::set_x0 (Coord x0)
{
	_points[0].x = x0;
}

void
Line::set_y0 (Coord y0)
{
	_points[0].y = y0;
}

void
Line::set_x1 (Coord x1)
{
	_points[1].x = x1;
}

void
Line::set_y1 (Coord y1)
{
	_points[1].y = y1;
}

