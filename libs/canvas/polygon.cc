#include "canvas/polygon.h"

using namespace ArdourCanvas;

Polygon::Polygon (Group* parent)
	: Item (parent)
	, PolyItem (parent)
	, Outline (parent)
	, Fill (parent)
{

}

void
Polygon::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	setup_outline_context (context);
	render_path (area, context);

	if (!_points.empty ()) {
		context->move_to (_points.front().x, _points.front().y);
	}
	
	context->fill ();
}
