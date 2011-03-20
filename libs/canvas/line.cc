#include <algorithm>
#include <cairomm/context.h>
#include "pbd/xml++.h"
#include "pbd/compose.h"
#include "canvas/line.h"
#include "canvas/types.h"
#include "canvas/debug.h"
#include "canvas/utils.h"

using namespace std;
using namespace ArdourCanvas;

Line::Line (Group* parent)
	: Item (parent)
	, Outline (parent)
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

	bbox = bbox.expand (_outline_width / 2);

	_bounding_box = bbox;
	_bounding_box_dirty = false;
}

void
Line::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	setup_outline_context (context);

	context->move_to (_points[0].x, _points[0].y);
	context->line_to (_points[1].x, _points[1].y);
	context->stroke ();
}

void
Line::set (Duple a, Duple b)
{
	begin_change ();

	_points[0] = a;
	_points[1] = b;

	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: line change\n");
}

void
Line::set_x0 (Coord x0)
{
	begin_change ();
	
	_points[0].x = x0;

	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: line change\n");
}

void
Line::set_y0 (Coord y0)
{
	begin_change ();

	_points[0].y = y0;

	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: line change\n");
}

void
Line::set_x1 (Coord x1)
{
	begin_change ();

	_points[1].x = x1;

	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: line change\n");
}

void
Line::set_y1 (Coord y1)
{
	begin_change ();

	_points[1].y = y1;

	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: line change\n");
}

XMLNode *
Line::get_state () const
{
	XMLNode* node = new XMLNode ("Line");
	node->add_property ("x0", string_compose ("%1", _points[0].x));
	node->add_property ("y0", string_compose ("%1", _points[0].y));
	node->add_property ("x1", string_compose ("%1", _points[1].x));
	node->add_property ("y1", string_compose ("%1", _points[1].y));

	add_item_state (node);
	add_outline_state (node);
	return node;
}

void
Line::set_state (XMLNode const * node)
{
	_points[0].x = atof (node->property("x0")->value().c_str());
	_points[0].y = atof (node->property("y0")->value().c_str());
	_points[1].x = atof (node->property("x1")->value().c_str());
	_points[1].y = atof (node->property("y1")->value().c_str());

	set_item_state (node);
	set_outline_state (node);

	_bounding_box_dirty = true;
}
