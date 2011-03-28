#include <iostream>
#include <cairomm/context.h>
#include "pbd/stacktrace.h"
#include "pbd/xml++.h"
#include "pbd/compose.h"
#include "canvas/rectangle.h"
#include "canvas/debug.h"
#include "canvas/utils.h"

using namespace std;
using namespace ArdourCanvas;

Rectangle::Rectangle (Group* parent)
	: Item (parent)
	, Outline (parent)
	, Fill (parent)
	, _outline_what ((What) (LEFT | RIGHT | TOP | BOTTOM))
{

}

Rectangle::Rectangle (Group* parent, Rect const & rect)
	: Item (parent)
	, Outline (parent)
	, Fill (parent)
	, _rect (rect)
	, _outline_what ((What) (LEFT | RIGHT | TOP | BOTTOM))
{
	
}

void
Rectangle::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	if (_fill) {
		setup_fill_context (context);
		context->rectangle (_rect.x0, _rect.y0, _rect.width(), _rect.height());
		context->fill ();
	}

	if (_outline) {
		if (_outline_what & LEFT) {
			context->move_to (_rect.x0, _rect.x0);
			context->line_to (_rect.x0, _rect.y1);
		}
		
		if (_outline_what & BOTTOM) {
			context->move_to (_rect.x0, _rect.y1);
			context->line_to (_rect.x1, _rect.y1);
		}
		
		if (_outline_what & RIGHT) {
			context->move_to (_rect.x1, _rect.y0);
			context->line_to (_rect.x1, _rect.y1);
		}
		
		if (_outline_what & TOP) {
			context->move_to (_rect.x0, _rect.y0);
			context->line_to (_rect.x0, _rect.y1);
		}
		
		setup_outline_context (context);
		context->stroke ();
	}
}

void
Rectangle::compute_bounding_box () const
{
	_bounding_box = boost::optional<Rect> (_rect.expand (_outline_width / 2));
	
	_bounding_box_dirty = false;
}

void
Rectangle::set (Rect const & r)
{
	/* We don't update the bounding box here; it's just
	   as cheap to do it when asked.
	*/
	
	begin_change ();
	
	_rect = r;
	fix_rect ();
	
	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: rectangle change (set)\n");
}

void
Rectangle::set_x0 (Coord x0)
{
	begin_change ();

	_rect.x0 = x0;
	fix_rect ();

	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: rectangle change (x0)\n");
}

void
Rectangle::set_y0 (Coord y0)
{
	begin_change ();
	
	_rect.y0 = y0;
	fix_rect ();

	_bounding_box_dirty = true;
	end_change();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: rectangle change (y0)\n");
}

void
Rectangle::set_x1 (Coord x1)
{
	begin_change ();
	
	_rect.x1 = x1;
	fix_rect ();

	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: rectangle change (x1)\n");
}

void
Rectangle::set_y1 (Coord y1)
{
	begin_change ();

	_rect.y1 = y1;
	fix_rect ();

	_bounding_box_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: rectangle change (y1)\n");
}

void
Rectangle::set_outline_what (What what)
{
	begin_change ();
	
	_outline_what = what;

	end_change ();
}

void
Rectangle::set_outline_what (int what)
{
	set_outline_what ((What) what);
}

void
Rectangle::fix_rect ()
{
	Rect r;
	
	r.x0 = min (_rect.x0, _rect.x1);
	r.y0 = min (_rect.y0, _rect.y1);
	r.x1 = max (_rect.x0, _rect.x1);
	r.y1 = max (_rect.y0, _rect.y1);

	_rect = r;
}

XMLNode *
Rectangle::get_state () const
{
	XMLNode* node = new XMLNode ("Rectangle");
#ifdef CANVAS_DEBUG
	node->add_property ("name", name);
#endif	
	node->add_property ("x0", string_compose ("%1", _rect.x0));
	node->add_property ("y0", string_compose ("%1", _rect.y0));
	node->add_property ("x1", string_compose ("%1", _rect.x1));
	node->add_property ("y1", string_compose ("%1", _rect.y1));
	node->add_property ("outline-what", string_compose ("%1", _outline_what));

	add_item_state (node);
	add_outline_state (node);
	add_fill_state (node);
	return node;
}

void
Rectangle::set_state (XMLNode const * node)
{
	_rect.x0 = atof (node->property("x0")->value().c_str());
	_rect.y0 = atof (node->property("y0")->value().c_str());
	_rect.x1 = atof (node->property("x1")->value().c_str());
	_rect.y1 = atof (node->property("y1")->value().c_str());
	_outline_what = (What) atoi (node->property("outline-what")->value().c_str());

	set_item_state (node);
	set_outline_state (node);
	set_fill_state (node);

	_bounding_box_dirty = true;
}
