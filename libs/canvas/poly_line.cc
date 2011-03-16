#include "canvas/poly_line.h"

using namespace ArdourCanvas;

PolyLine::PolyLine (Group* parent)
	: Item (parent)
	, PolyItem (parent)
	, Outline (parent)
{

}

void
PolyLine::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	setup_context (context);
	render_path (area, context);
	context->stroke ();
}
