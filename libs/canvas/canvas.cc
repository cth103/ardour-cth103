#include <cassert>
#include "canvas/canvas.h"

using namespace std;
using namespace ArdourCanvas;

Canvas::Canvas ()
	: _root ((Group *) 0)
{

}

Canvas*
Canvas::create_image ()
{
	ImageCanvas* c = new ImageCanvas;
	c->_surface = c->create_surface ();
	c->_context = c->create_context ();
	return c;
}

Cairo::RefPtr<Cairo::Context>
Canvas::create_context ()
{
	assert (_surface);
	return Cairo::Context::create (_surface);
}

/** @param area Area in canvas coordinates */
void
Canvas::render (Rect const & area) const
{
	_root.render (_root.bounding_box().intersection (area), _context);
}

ImageCanvas::ImageCanvas ()
{

}
		
Cairo::RefPtr<Cairo::Surface>
ImageCanvas::create_surface ()
{
	return Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, 1024, 1024);
}

void
ImageCanvas::write_to_png (string const & f)
{
	assert (_surface);
	_surface->write_to_png (f);
}
