#include <iostream>
#include <cairomm/context.h>
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
	context->move_to (_rect.x0, _rect.y0);

	if (_outline_what & LEFT) {
		context->line_to (_rect.x0, _rect.y1);
	} else {
		context->move_to (_rect.x0, _rect.y1);
	}

	if (_outline_what & BOTTOM) {
		context->line_to (_rect.x1, _rect.y1);
	} else {
		context->move_to (_rect.x1, _rect.y1);
	}

	if (_outline_what & RIGHT) {
		context->line_to (_rect.x1, _rect.y0);
	} else {
		context->move_to (_rect.x1, _rect.y0);
	}

	if (_outline_what & TOP) {
		context->line_to (_rect.x0, _rect.y0);
	} else {
		context->move_to (_rect.x0, _rect.y0);
	}
		
	if (_fill) {
		setup_fill_context (context);
		context->fill_preserve ();
	}
	
	setup_outline_context (context);
	context->stroke ();

	Debug::instance()->render_object_count++;
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
}

void
Rectangle::set_x0 (Coord x0)
{
	begin_change ();

	_rect.x0 = x0;
	fix_rect ();

	_bounding_box_dirty = true;
	end_change ();
}

void
Rectangle::set_y0 (Coord y0)
{
	begin_change ();
	
	_rect.y0 = y0;
	fix_rect ();

	_bounding_box_dirty = true;
	end_change();
}

void
Rectangle::set_x1 (Coord x1)
{
	begin_change ();
	
	_rect.x1 = x1;
	fix_rect ();

	_bounding_box_dirty = true;
	end_change ();
}

void
Rectangle::set_y1 (Coord y1)
{
	begin_change ();

	_rect.y1 = y1;
	fix_rect ();

	_bounding_box_dirty = true;
	end_change ();
}

void
Rectangle::set_outline_what (What what)
{
	begin_change ();
	
	_outline_what = what;

	end_change ();
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
