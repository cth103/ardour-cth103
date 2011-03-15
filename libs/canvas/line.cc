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

void
Line::compute_bounding_box () const
{
	Rect bbox;
	
	bbox.x0 = min (_points[0].x, _points[1].x);
	bbox.y0 = min (_points[0].y, _points[1].y);
	bbox.x1 = max (_points[0].x, _points[1].x);
	bbox.y1 = max (_points[0].y, _points[1].y);

	_bounding_box = bbox;
	_bounding_box_dirty = false;
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
	begin_change ();
	
	_points[0] = a;
	_points[1] = b;

	_bounding_box_dirty = true;
	end_change ();
}

void
Line::set_x0 (Coord x0)
{
	begin_change ();
	
	_points[0].x = x0;

	_bounding_box_dirty = true;
	end_change ();
}

void
Line::set_y0 (Coord y0)
{
	begin_change ();

	_points[0].y = y0;

	_bounding_box_dirty = true;
	end_change ();
}

void
Line::set_x1 (Coord x1)
{
	begin_change ();

	_points[1].x = x1;

	_bounding_box_dirty = true;
	end_change ();
}

void
Line::set_y1 (Coord y1)
{
	begin_change ();

	_points[1].y = y1;

	_bounding_box_dirty = true;
	end_change ();
}

