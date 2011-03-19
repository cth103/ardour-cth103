#include <cassert>
#include <gtkmm/adjustment.h>
#include "pbd/compose.h"
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

	cout << "Root bounding box: " << root_bbox.get() << "\n";
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

bool
GtkCanvas::button_press_handler (GdkEventButton* ev)
{
	DEBUG_TRACE (PBD::DEBUG::CanvasEvents, string_compose ("canvas button press %1 %1\n", ev->x, ev->y));
	return deliver_event (Duple (ev->x, ev->y), reinterpret_cast<GdkEvent*> (ev));
}

bool
GtkCanvas::deliver_event (Duple point, GdkEvent* event)
{
	list<Item*> items;
	_root.add_items_at_point (point, items);
	if (items.empty()) {
		return false;
	}
	
	if (PBD::debug_bits & PBD::DEBUG::CanvasEvents) {
		for (list<Item*>::reverse_iterator i = items.rbegin(); i != items.rend(); ++i) {
			DEBUG_TRACE (PBD::DEBUG::CanvasEvents, string_compose ("%1\n", (*i)->name.empty() ? "[unknown]" : (*i)->name));
		}
	}

	DEBUG_TRACE (PBD::DEBUG::CanvasEvents, "canvas deliver event to top of:\n");
	list<Item*>::reverse_iterator i = items.rbegin ();
	while (i != items.rend()) {
		if ((*i)->Event (event)) {
			DEBUG_TRACE (PBD::DEBUG::CanvasEvents, string_compose ("canvas event handled by %1\n", (*i)->name.empty() ? "[unknown]" : (*i)->name));
			return true;
		}
		DEBUG_TRACE (PBD::DEBUG::CanvasEvents, string_compose ("canvas event ignored by %1\n", (*i)->name.empty() ? "[unknown]" : (*i)->name));
		++i;
	}

	
	if (PBD::debug_bits & PBD::DEBUG::CanvasEvents) {
		while (i != items.rend()) {
			DEBUG_TRACE (PBD::DEBUG::CanvasEvents, string_compose ("canvas event not seen by %1\n", (*i)->name.empty() ? "[unknown]" : (*i)->name));
			--i;
		}
	}
	
	return false;
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
	add_events (Gdk::BUTTON_PRESS_MASK);
}

bool
GtkCanvasDrawingArea::on_expose_event (GdkEventExpose* ev)
{
	cout << "GtkCanvasDrawingArea: exposed x=" << ev->area.x << " width=" << ev->area.width << "\n";
	Cairo::RefPtr<Cairo::Context> c = get_window()->create_cairo_context ();
	render (Rect (ev->area.x, ev->area.y, ev->area.x + ev->area.width, ev->area.y + ev->area.height), c);
	return true;
}

bool
GtkCanvasDrawingArea::on_button_press_event (GdkEventButton* ev)
{
	return button_press_handler (ev);
}

void
GtkCanvasDrawingArea::request_redraw (Rect const & area)
{
	queue_draw_area (floor (area.x0), floor (area.y0), ceil (area.x1) - floor (area.x0), ceil (area.y1) - floor (area.y0));
}

void
GtkCanvasDrawingArea::request_size (Duple size)
{
	set_size_request (size.x, size.y);
}

GtkCanvasViewport::GtkCanvasViewport (Gtk::Adjustment& hadj, Gtk::Adjustment& vadj)
	: Viewport (hadj, vadj)
{
	add (_canvas);
}

void
GtkCanvasViewport::on_size_request (Gtk::Requisition* req)
{
	req->width = 128;
	req->height = 128;
}
