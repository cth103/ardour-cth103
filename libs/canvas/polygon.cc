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
	setup_context (context);
	render_path (area, context);
	context->fill ();
}
