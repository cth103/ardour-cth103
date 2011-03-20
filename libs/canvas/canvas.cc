#include <cassert>
#include <gtkmm/adjustment.h>
#include "pbd/xml++.h"
#include "pbd/compose.h"
#include "canvas/canvas.h"
#include "canvas/debug.h"

using namespace std;
using namespace ArdourCanvas;

Canvas::Canvas ()
	: _root (this)
{

}

Canvas::Canvas (XMLTree const * tree)
	: _root (this)
{
	/* XXX: little bit hacky */
	_root.set_state (tree->root()->child ("Group"));
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

XMLTree *
Canvas::get_state ()
{
	XMLTree* tree = new XMLTree ();
	XMLNode* node = new XMLNode ("Canvas");
	node->add_child_nocopy (*_root.get_state ());

	tree->set_root (node);
	return tree;
}


GtkCanvas::GtkCanvas ()
	: _current_item (0)
	, _grabbed_item (0)
{
	add_events (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

GtkCanvas::GtkCanvas (XMLTree const * tree)
	: Canvas (tree)
	, _current_item (0)
	, _grabbed_item (0)
{
	add_events (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

bool
GtkCanvas::button_handler (GdkEventButton* ev)
{
	DEBUG_TRACE (PBD::DEBUG::CanvasEvents, string_compose ("canvas button press %1 %1\n", ev->x, ev->y));
	return deliver_event (Duple (ev->x, ev->y), reinterpret_cast<GdkEvent*> (ev));
}

bool
GtkCanvas::motion_notify_handler (GdkEventMotion* ev)
{
	/* XXX: do this every time the mouse moves? really? */

	Duple point (ev->x, ev->y);
	
	vector<Item const *> items;
	_root.add_items_at_point (point, items);
	Item const * new_item = items.empty () ? 0 : items.back ();

	if (new_item != _current_item) {
		GdkEventCrossing synth_event;

		if (_current_item) {
			synth_event.type = GDK_LEAVE_NOTIFY;
			synth_event.x = ev->x;
			synth_event.y = ev->y;
			_current_item->Event (reinterpret_cast<GdkEvent*> (&synth_event));
		}

		if (new_item) {
			synth_event.type = GDK_ENTER_NOTIFY;
			synth_event.x = ev->x;
			synth_event.y = ev->y;
			new_item->Event (reinterpret_cast<GdkEvent*> (&synth_event));
		}

		_current_item = new_item;
	}

	return deliver_event (point, reinterpret_cast<GdkEvent*> (ev));
}

bool
GtkCanvas::deliver_event (Duple point, GdkEvent* event)
{
	if (_grabbed_item) {
		return _grabbed_item->Event (event);
	}
	
	vector<Item const *> items;
	_root.add_items_at_point (point, items);
	if (items.empty()) {
		return false;
	}
	
	vector<Item const *>::reverse_iterator i = items.rbegin ();
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
			++i;
		}
	}
	
	return false;
}

ImageCanvas::ImageCanvas (Duple size)
	: _surface (Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, size.x, size.y))
{
	_context = Cairo::Context::create (_surface);
}

ImageCanvas::ImageCanvas (XMLTree const * tree, Duple size)
	: Canvas (tree)
	, _surface (Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, size.x, size.y))
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

bool
GtkCanvas::on_expose_event (GdkEventExpose* ev)
{
	cout << "GtkCanvas: exposed x=" << ev->area.x << " width=" << ev->area.width << "\n";
	Cairo::RefPtr<Cairo::Context> c = get_window()->create_cairo_context ();
	render (Rect (ev->area.x, ev->area.y, ev->area.x + ev->area.width, ev->area.y + ev->area.height), c);
	return true;
}

bool
GtkCanvas::on_button_press_event (GdkEventButton* ev)
{
	return button_handler (ev);
}

bool
GtkCanvas::on_button_release_event (GdkEventButton* ev)
{
	return button_handler (ev);
}

bool
GtkCanvas::on_motion_notify_event (GdkEventMotion* ev)
{
	return motion_notify_handler (ev);
}

void
GtkCanvas::request_redraw (Rect const & area)
{
	queue_draw_area (floor (area.x0), floor (area.y0), ceil (area.x1) - floor (area.x0), ceil (area.y1) - floor (area.y0));
}

void
GtkCanvas::request_size (Duple size)
{
	set_size_request (size.x, size.y);
}

void
GtkCanvas::grab (Item* item)
{
	/* XXX: should this be doing gdk_pointer_grab? */
	_grabbed_item = item;
}

void
GtkCanvas::ungrab ()
{
	/* XXX: should this be doing gdk_pointer_ungrab? */
	_grabbed_item = 0;
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
