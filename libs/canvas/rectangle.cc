#include <iostream>
#include <cairomm/context.h>
#include "canvas/rectangle.h"
#include "canvas/debug.h"
#include "canvas/utils.h"

using namespace std;
using namespace ArdourCanvas;

Rectangle::Rectangle (Group* parent)
	: Item (parent)
{

}

Rectangle::Rectangle (Group* parent, Rect const & rect)
	: Item (parent)
	, _rect (rect)
{
	
}

void
Rectangle::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	if (_watch) {
		cout << "* RENDER " << _rect << "\n";
	}

	setup_context (context);
	
	context->rectangle (_rect.x0, _rect.y0, _rect.width(), _rect.height());
	context->stroke ();

	Debug::instance()->render_object_count++;
}

boost::optional<Rect>
Rectangle::bounding_box () const
{
	return boost::optional<Rect> (_rect);
}

void
Rectangle::set (Rect const & r)
{
	_rect = r;
}

void
Rectangle::set_x0 (Coord x0)
{
	_rect.x0 = x0;
}

void
Rectangle::set_y0 (Coord y0)
{
	_rect.y0 = y0;
}

void
Rectangle::set_x1 (Coord x1)
{
	_rect.x1 = x1;
}

void
Rectangle::set_y1 (Coord y1)
{
	_rect.y1 = y1;
}

