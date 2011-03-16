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
{

}

Rectangle::Rectangle (Group* parent, Rect const & rect)
	: Item (parent)
	, Outline (parent)
	, Fill (parent)
	, _rect (rect)
{
	
}

void
Rectangle::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	setup_context (context);
	
	context->rectangle (_rect.x0, _rect.y0, _rect.width(), _rect.height());
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
	
	_bounding_box_dirty = true;
	end_change ();
}

void
Rectangle::set_x0 (Coord x0)
{
	begin_change ();

	_rect.x0 = x0;

	_bounding_box_dirty = true;
	end_change ();
}

void
Rectangle::set_y0 (Coord y0)
{
	begin_change ();
	
	_rect.y0 = y0;
	
	_bounding_box_dirty = true;
	end_change();
}

void
Rectangle::set_x1 (Coord x1)
{
	begin_change ();
	
	_rect.x1 = x1;

	_bounding_box_dirty = true;
	end_change ();
}

void
Rectangle::set_y1 (Coord y1)
{
	begin_change ();

	_rect.y1 = y1;

	_bounding_box_dirty = true;
	end_change ();
}

