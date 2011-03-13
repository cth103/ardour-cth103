#include <cassert>
#include "canvas/canvas.h"

using namespace std;
using namespace ArdourCanvas;

Canvas::Canvas ()
	: _root ((Group *) 0)
{

}

/** @param area Area in canvas coordinates */
void
Canvas::render (Rect const & area, Cairo::RefPtr<Cairo::Context> const & context) const
{
	_root.render (_root.bounding_box().intersection (area), context);
}

ImageCanvas::ImageCanvas ()
	: _surface (Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 1024, 1024))
{
	_context = Cairo::Context::create (_surface);
}

void
ImageCanvas::render_to_image (Rect const & area) const
{
	render (area, _context);
}

void
ImageCanvas::write_to_png (string const & f)
{
	assert (_surface);
	_surface->write_to_png (f);
}

GtkCanvas::GtkCanvas ()
{
	
}
