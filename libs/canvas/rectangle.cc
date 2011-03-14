#include <iostream>
#include <cairomm/context.h>
#include "canvas/rectangle.h"
#include "canvas/debug.h"

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
	context->set_source_rgb (1, 0, 0);
	context->rectangle (_rect.x0, _rect.y0, _rect.width(), _rect.height());
	context->stroke ();

	Debug::instance()->render_object_count++;
}

Rect
Rectangle::bounding_box () const
{
	return _rect;
}

void
Rectangle::set (Rect const & r)
{
	_rect = r;
}