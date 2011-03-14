#include <cassert>
#include "canvas/canvas.h"
#include "canvas/debug.h"

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
	Debug::instance()->render_object_count = 0;
		
	context->rectangle (area.x0, area.y0, area.width(), area.height());
	context->clip ();
	
	boost::optional<Rect> draw = _root.bounding_box().intersection (area);
	if (draw) {
		_root.render (*draw, context);
	}

	cout << "Rendered: " << Debug::instance()->render_object_count << "\n";
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

GtkCanvasDrawingArea::GtkCanvasDrawingArea ()
{
	
}

bool
GtkCanvasDrawingArea::on_expose_event (GdkEventExpose* ev)
{
	Cairo::RefPtr<Cairo::Context> c = get_window()->create_cairo_context ();
	_canvas.render (Rect (ev->area.x, ev->area.y, ev->area.x + ev->area.width, ev->area.y + ev->area.height), c);
	return true;
}
