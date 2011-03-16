#include <cassert>
#include "canvas/canvas.h"
#include "canvas/debug.h"

using namespace std;
using namespace ArdourCanvas;

Canvas::Canvas ()
	: _root (this)
{

}

/** @param area Area in canvas coordinates */
void
Canvas::render (Rect const & area, Cairo::RefPtr<Cairo::Context> const & context) const
{
	Debug::instance()->render_object_count = 0;
		
	context->rectangle (area.x0, area.y0, area.width(), area.height());
	context->clip ();

	boost::optional<Rect> root_bbox = _root.bounding_box();
	if (!root_bbox) {
		return;
	}

	boost::optional<Rect> draw = root_bbox.get().intersection (area);
	if (draw) {
		_root.render (*draw, context);
	}

	cout << "Rendered: " << Debug::instance()->render_object_count << "\n";
}

void
Canvas::item_changed (Item* item, boost::optional<Rect> pre_change_bounding_box)
{
	/* XXX: could be more efficient */
	if (pre_change_bounding_box) {
		queue_draw_item_area (item, pre_change_bounding_box.get ());
	}

	boost::optional<Rect> post_change_bounding_box = item->bounding_box ();
	if (post_change_bounding_box) {
		queue_draw_item_area (item, post_change_bounding_box.get ());
	}
}

void
Canvas::item_moved (Item* item, boost::optional<Rect> pre_change_parent_bounding_box)
{
	/* XXX: could be more efficient */
	if (pre_change_parent_bounding_box) {
		queue_draw_item_area (item->parent(), pre_change_parent_bounding_box.get ());
	}

	boost::optional<Rect> post_change_bounding_box = item->bounding_box ();
	if (post_change_bounding_box) {
		queue_draw_item_area (item, post_change_bounding_box.get ());
	}
}

void
Canvas::queue_draw_item_area (Item* item, Rect area)
{
	Item* i = item;
	while (i) {
		area = i->item_to_parent (area);
		i = i->parent ();
	}

	request_redraw (area);
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
	render (Rect (ev->area.x, ev->area.y, ev->area.x + ev->area.width, ev->area.y + ev->area.height), c);
	return true;
}

void
GtkCanvasDrawingArea::request_redraw (Rect const & area)
{
	queue_draw_area (floor (area.x0), floor (area.y0), ceil (area.x1) - floor (area.x0), ceil (area.y1) - floor (area.y0));
}
