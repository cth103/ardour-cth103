/*
    Copyright (C) 2011 Paul Davis
    Author: Carl Hetherington <cth@carlh.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/** @file  canvas/canvas.cc
 *  @brief Implementation of the main canvas classes.
 */

#include <cassert>
#include <gtkmm/adjustment.h>
#include "pbd/xml++.h"
#include "pbd/compose.h"
#include "pbd/stacktrace.h"
#include "canvas/canvas.h"
#include "canvas/debug.h"

using namespace std;
using namespace ArdourCanvas;

/** Construct a Tile.  This creates the blank bitmap and marks
 *  it dirty, so that it is redrawn when required.
 *  @param canvas Our parent canvas.
 *  @param tx The x index of this tile within the canvas.
 *  @param ty The y index of this tile within the canvas.
 *  @param size The width and height of this tile, in pixels.
 */
Tile::Tile (Canvas const * canvas, int tx, int ty, int size)
	: _canvas (canvas)
	, _tx (tx)
	, _ty (ty)
	, _dirty (true)
{
	_surface = Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, size, size);
	_context = Cairo::Context::create (_surface);
}

/** If we are marked as `dirty', ask the canvas to redraw this tile
 *  onto our bitmap.
 */
void
Tile::render ()
{
	if (!_dirty) {
		return;
	}

	/* blank the tile */
	_context->save ();
	_context->set_operator (Cairo::OPERATOR_CLEAR);
	_context->rectangle (0, 0, _surface->get_width(), _surface->get_height());
	_context->fill ();
	_context->restore ();

	/* ask the canvas to draw onto it */
	_canvas->render_to_tile (_context, _tx, _ty);
	_dirty = false;

	_canvas->tile_render_count++;
}

/** Mark this tile as being in need of a repaint */
void
Tile::set_dirty ()
{
	_dirty = true;
}

/** Construct a new Canvas */
Canvas::Canvas ()
	: _root (this)
	, _updates_suspended (false)
#ifdef CANVAS_DEBUG	  
	, _log_renders (false)
#endif	  
	, _tile_size (64)
{
#ifdef CANVAS_DEBUG	
	set_epoch ();
#endif	
}

/** Construct a new Canvas from an XML tree.
 *  @param tree XML Tree.
 */
Canvas::Canvas (XMLTree const * tree)
	: _root (this)
	, _updates_suspended (false)
#ifdef CANVAS_DEBUG	  
	, _log_renders (false)
#endif	  
	, _tile_size (64)
{
#ifdef CANVAS_DEBUG	  
	set_epoch ();
#endif	
	
	/* XXX: little bit hacky */
	_root.set_state (tree->root()->child ("Group"));

	/* Load renders list from the XML */
	XMLNodeList const & children = tree->root()->children ();
	for (XMLNodeList::const_iterator i = children.begin(); i != children.end(); ++i) {
		if ((*i)->name() == ("Render")) {
			_renders.push_back (
				Rect (
					atof ((*i)->property ("x0")->value().c_str()),
					atof ((*i)->property ("y0")->value().c_str()),
					atof ((*i)->property ("x1")->value().c_str()),
					atof ((*i)->property ("x1")->value().c_str())
					)
				);
		}
	}
}

/** Ensure that a tile exists at a given pair of indices.
 *  @param tx Tile x index.
 *  @param ty Tile y index.
 */
void
Canvas::ensure_tile (int tx, int ty) const
{
	/* Grow the array */
	
	if (int (_tiles.size()) <= tx) {
		_tiles.resize (tx + 1);
	}

	for (vector<vector<boost::shared_ptr<Tile> > >::iterator i = _tiles.begin(); i != _tiles.end(); ++i) {
		if (int (i->size()) <= ty) {
			i->resize (ty + 1);
		}
	}

	/* Create a tile if we don't already have one */
	
	if (_tiles[tx][ty] == 0) {
		_tiles[tx][ty].reset (new Tile (this, tx, ty, _tile_size));
	}
}

/** Convert an area to a range of tiles that are required to completely cover it.
 *  @param area Area in canvas coordinates.
 *  @param tx0 Set to the lower x tile index (inclusive).
 *  @param ty0 Set to the lower y tile index (inclusive).
 *  @param tx1 Set to the upper x tile index (inclusive).
 *  @param ty1 Set to the upper y tile index (inclusive).
 */
void
Canvas::area_to_tiles (Rect const & area, int& tx0, int& ty0, int& tx1, int& ty1) const
{
	if (area.x0 == COORD_MAX) {
		tx0 = INT_MAX;
	} else {
		tx0 = area.x0 / _tile_size;
	}

	if (area.x1 == COORD_MAX) {
		tx1 = INT_MAX;
	} else {
		/* round up */
		tx1 = (area.x1 + (_tile_size / 2)) / _tile_size;
	}

	if (area.y0 == COORD_MAX) {
		ty0 = INT_MAX;
	} else {
		ty0 = area.y0 / _tile_size;
	}

	if (area.y1 == COORD_MAX) {
		ty1 = INT_MAX;
	} else {
		/* round up */
		ty1 = (area.y1 + (_tile_size / 2)) / _tile_size;
	}
}

/** Render an area of the canvas using our tiles, creating and updating tiles as we go */
void
Canvas::render_from_tiles (Rect const & area, Cairo::RefPtr<Cairo::Context> const & context) const
{
	tile_render_count = 0;
	
#ifdef CANVAS_DEBUG	
	if (_log_renders) {
		_renders.push_back (area);
	}
#endif	
	
	int tx0, ty0, tx1, ty1;
	area_to_tiles (area, tx0, ty0, tx1, ty1);

	context->save ();

	/* Clip to the requested area */
	context->rectangle (area.x0, area.y0, area.width(), area.height());
	context->clip ();

	/* Build and plot the tiles */
	for (int x = tx0; x <= tx1; ++x) {
		for (int y = ty0; y <= ty1; ++y) {
			ensure_tile (x, y);
			_tiles[x][y]->render ();
			context->set_source (_tiles[x][y]->surface (), x * _tile_size, y * _tile_size);
			context->paint ();
		}
	}
	
	context->restore ();

//	cout << "Rendered " << tile_render_count << " tiles.\n";
}

/** Render an area of the canvas to a tile, by drawing the required items.
 *  @param context Tile's context to draw onto.
 *  @param tx Tile's x index.
 *  @param ty Tile's y index.
 */
void
Canvas::render_to_tile (Cairo::RefPtr<Cairo::Context> context, int tx, int ty) const
{
	boost::optional<Rect> root_bbox = _root.bbox ();
	if (!root_bbox) {
		return;
	}

	/* Work out what we need to draw by intersecting the canvas root bounding box
	 * with the requested tile's area.
	 */
	Rect area (tx * _tile_size, ty * _tile_size, (tx + 1) * _tile_size, (ty + 1) * _tile_size);
	boost::optional<Rect> draw = root_bbox.get().intersection (area);

	if (!draw) {
		return;
	}

	context->save ();
	/* clip to the tile */
	context->rectangle (0, 0, _tile_size, _tile_size);
	context->clip ();
	/* translate the context so that the required canvas area starts at (0, 0) on the tile */
	context->translate (-area.x0, -area.y0);
	_root.render (*draw, context);
	context->restore ();
}

/** Called when an item has changed.
 *  @param item Item that has changed.
 *  @param pre_parent_bbox The bounding box of item before the change,
 *  in the parent's coordinates.
 */
void
Canvas::item_changed (Item* item, boost::optional<Rect> pre_parent_bbox)
{
	if (_updates_suspended) {
		return;
	}

	/* get the item's bbox in its parents coordinates */
	boost::optional<Rect> parent_bbox;
	if (item->bbox ()) {
		parent_bbox = item->item_to_parent (item->bbox().get ());
	}

	if (pre_parent_bbox) {

		/* there was a pre-change bbox */
		
		if (!parent_bbox || parent_bbox.get() != pre_parent_bbox.get()) {
			/* the pre-change bbox is different to the post-change one; mark it dirty */
			mark_item_area_dirty (item->parent (), pre_parent_bbox.get ());
		}
	}

	if (parent_bbox) {
		/* there is a post-change bbox; mark it dirty */
		mark_item_area_dirty (item->parent (), parent_bbox.get ());
	}
}

/** Request a redraw of a particular area in an item's coordinates.
 *  @param item Item.
 *  @param area Area to redraw in the item's coordinates.
 */
void
Canvas::mark_item_area_dirty (Item* item, Rect area)
{
	if (_tiles.empty ()) {
		return;
	}
	
	Rect const canvas_area = item->item_to_canvas (area);
	
	/* Mark the appropriate tiles dirty */
	int tx0, ty0, tx1, ty1;
	area_to_tiles (canvas_area, tx0, ty0, tx1, ty1);

	tx0 = min (tx0, int (_tiles.size() - 1));
	tx1 = min (tx1, int (_tiles.size() - 1));

	for (int x = tx0; x <= tx1; ++x) {

		for (int y = ty0; y <= ty1; ++y) {

			if (y >= int (_tiles[x].size())) {
				break;
			}
			
			if (_tiles[x][y]) {
				_tiles[x][y]->set_dirty ();
			}
		}
	}

	/* Request a redraw of the canvas, which will re-render the tiles too */
	request_redraw (canvas_area);
}

/** Suspend all updates to the canvas */
void
Canvas::suspend_updates ()
{
	_updates_suspended = true;
}

/** Resume updates and redraw the whole visible canvas area */
void
Canvas::resume_updates ()
{
	_updates_suspended = false;
	for (vector<vector<boost::shared_ptr<Tile> > >::iterator i = _tiles.begin(); i != _tiles.end(); ++i) {
		for (vector<boost::shared_ptr<Tile> >::iterator j = i->begin(); j != i->end(); ++j) {
			if (*j) {
				(*j)->set_dirty ();
			}
		}
	}

	boost::optional<Rect> bbox = _root.bbox ();
	if (bbox) {
		request_redraw (_root.item_to_canvas (bbox.get()));
	}
}

/** @return An XML description of the canvas and its objects */
XMLTree *
Canvas::get_state () const
{
	XMLTree* tree = new XMLTree ();
	XMLNode* node = new XMLNode ("Canvas");
	node->add_child_nocopy (*_root.get_state ());

	for (list<Rect>::const_iterator i = _renders.begin(); i != _renders.end(); ++i) {
		XMLNode* render = new XMLNode ("Render");
		render->add_property ("x0", string_compose ("%1", i->x0));
		render->add_property ("y0", string_compose ("%1", i->y0));
		render->add_property ("x1", string_compose ("%1", i->x1));
		render->add_property ("y1", string_compose ("%1", i->y1));
		node->add_child_nocopy (*render);
	}

	tree->set_root (node);
	return tree;
}

/** Construct a GtkCanvas */
GtkCanvas::GtkCanvas ()
	: _current_item (0)
	, _grabbed_item (0)
{
	/* these are the events we want to know about */
	add_events (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

/** Construct a GtkCanvas from an XML tree.
 *  @param tree XML Tree.
 */
GtkCanvas::GtkCanvas (XMLTree const * tree)
	: Canvas (tree)
	, _current_item (0)
	, _grabbed_item (0)
{
	/* these are the events we want to know about */
	add_events (Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK | Gdk::POINTER_MOTION_MASK);
}

/** Handler for button presses on the canvas.
 *  @param ev GDK event.
 */
bool
GtkCanvas::button_handler (GdkEventButton* ev)
{
	DEBUG_TRACE (PBD::DEBUG::CanvasEvents, string_compose ("canvas button press %1 %1\n", ev->x, ev->y));
	/* The Duple that we are passing in here is in canvas coordinates */
	return deliver_event (Duple (ev->x, ev->y), reinterpret_cast<GdkEvent*> (ev));
}

/** Handler for pointer motion events on the canvas.
 *  @param ev GDK event.
 *  @return true if the motion event was handled, otherwise false.
 */
bool
GtkCanvas::motion_notify_handler (GdkEventMotion* ev)
{
	if (_grabbed_item) {
		/* if we have a grabbed item, it gets just the motion event,
		   since no enter/leave events can have happened.
		*/
		return _grabbed_item->Event (reinterpret_cast<GdkEvent*> (ev));
	}

	/* This is in canvas coordinates */
	Duple point (ev->x, ev->y);

	/* find the items at the new mouse position */
	vector<Item const *> items;
	_root.add_items_at_point (point, items);

	Item const * new_item = items.empty() ? 0 : items.back ();

	if (_current_item && _current_item != new_item) {
		/* leave event */
		GdkEventCrossing leave_event;
		leave_event.type = GDK_LEAVE_NOTIFY;
		leave_event.x = ev->x;
		leave_event.y = ev->y;
		_current_item->Event (reinterpret_cast<GdkEvent*> (&leave_event));
	}

	if (new_item && _current_item != new_item) {
		/* enter event */
		GdkEventCrossing enter_event;
		enter_event.type = GDK_ENTER_NOTIFY;
		enter_event.x = ev->x;
		enter_event.y = ev->y;
		new_item->Event (reinterpret_cast<GdkEvent*> (&enter_event));
	}

	_current_item = new_item;

	/* Now deliver the motion event.  It may seem a little inefficient
	   to recompute the items under the event, but the enter notify/leave
	   events may have deleted canvas items so it is important to
	   recompute the list in deliver_event.
	*/
	return deliver_event (point, reinterpret_cast<GdkEvent*> (ev));
}

/** Deliver an event to the appropriate item; either the grabbed item, or
 *  one of the items underneath the event.
 *  @param point Position that the event has occurred at, in canvas coordinates.
 *  @param event The event.
 */
bool
GtkCanvas::deliver_event (Duple point, GdkEvent* event)
{
	if (_grabbed_item) {
		/* we have a grabbed item, so everything gets sent there */
		return _grabbed_item->Event (event);
	}

	/* find the items that exist at the event's position */
	vector<Item const *> items;
	_root.add_items_at_point (point, items);

	/* run through the items under the event, from top to bottom, until one claims the event */
	vector<Item const *>::const_reverse_iterator i = items.rbegin ();
	while (i != items.rend()) {

		if ((*i)->ignore_events ()) {
			++i;
			continue;
		}
		
		if ((*i)->Event (event)) {
			/* this item has just handled the event */
			DEBUG_TRACE (
				PBD::DEBUG::CanvasEvents,
				string_compose ("canvas event handled by %1\n", (*i)->name.empty() ? "[unknown]" : (*i)->name)
				);
			
			return true;
		}
		
		DEBUG_TRACE (
			PBD::DEBUG::CanvasEvents,
			string_compose ("canvas event ignored by %1\n", (*i)->name.empty() ? "[unknown]" : (*i)->name)
			);
		
		++i;
	}

	/* debugging */
	if (PBD::debug_bits & PBD::DEBUG::CanvasEvents) {
		while (i != items.rend()) {
			DEBUG_TRACE (PBD::DEBUG::CanvasEvents, string_compose ("canvas event not seen by %1\n", (*i)->name.empty() ? "[unknown]" : (*i)->name));
			++i;
		}
	}
	
	return false;
}

/** Called when an item is being destroyed.
 *  @param item Item being destroyed.
 *  @param bbox Last known bounding box of the item.
 */
void
GtkCanvas::item_going_away (Item* item, boost::optional<Rect> bbox)
{
	if (bbox && !_updates_suspended) {
		mark_item_area_dirty (item, bbox.get ());
	}
	
	if (_current_item == item) {
		_current_item = 0;
	}

	if (_grabbed_item == item) {
		_grabbed_item = 0;
	}
}

/** Construct an ImageCanvas.
 *  @param size Size in pixels.
 */
ImageCanvas::ImageCanvas (Duple size)
	: _surface (Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, size.x, size.y))
{
	_context = Cairo::Context::create (_surface);
}

/** Construct an ImageCanvas from an XML tree.
 *  @param tree XML Tree.
 *  @param size Size in pixels.
 */
ImageCanvas::ImageCanvas (XMLTree const * tree, Duple size)
	: Canvas (tree)
	, _surface (Cairo::ImageSurface::create (Cairo::FORMAT_ARGB32, size.x, size.y))
{
	_context = Cairo::Context::create (_surface);
}

/** Render the canvas to our pixbuf.
 *  @param area Area to render, in canvas coordinates.
 */
void
ImageCanvas::render_to_image (Rect const & area) const
{
	render_from_tiles (area, _context);
}

/** Write our pixbuf to a PNG file.
 *  @param f PNG file name.
 */
void
ImageCanvas::write_to_png (string const & f)
{
	assert (_surface);
	_surface->write_to_png (f);
}

/** @return Our Cairo context */
Cairo::RefPtr<Cairo::Context>
ImageCanvas::context ()
{
	return _context;
}

/** Handler for GDK expose events.
 *  @param ev Event.
 *  @return true if the event was handled.
 */
bool
GtkCanvas::on_expose_event (GdkEventExpose* ev)
{
	Cairo::RefPtr<Cairo::Context> c = get_window()->create_cairo_context ();
	render_from_tiles (Rect (ev->area.x, ev->area.y, ev->area.x + ev->area.width, ev->area.y + ev->area.height), c);
	return true;
}

/** @return Our Cairo context, or 0 if we don't have one */
Cairo::RefPtr<Cairo::Context>
GtkCanvas::context ()
{
	Glib::RefPtr<Gdk::Window> w = get_window ();
	if (!w) {
		return Cairo::RefPtr<Cairo::Context> ();
	}

	return w->create_cairo_context ();
}

/** Handler for GDK button press events.
 *  @param ev Event.
 *  @return true if the event was handled.
 */
bool
GtkCanvas::on_button_press_event (GdkEventButton* ev)
{
	/* Coordinates in the event will be canvas coordinates, correctly adjusted
	   for scroll if this GtkCanvas is in a GtkCanvasViewport.
	*/
	return button_handler (ev);
}

/** Handler for GDK button release events.
 *  @param ev Event.
 *  @return true if the event was handled.
 */
bool
GtkCanvas::on_button_release_event (GdkEventButton* ev)
{
	/* Coordinates in the event will be canvas coordinates, correctly adjusted
	   for scroll if this GtkCanvas is in a GtkCanvasViewport.
	*/
	return button_handler (ev);
}

/** Handler for GDK motion events.
 *  @param ev Event.
 *  @return true if the event was handled.
 */
bool
GtkCanvas::on_motion_notify_event (GdkEventMotion* ev)
{
	/* Coordinates in the event will be canvas coordinates, correctly adjusted
	   for scroll if this GtkCanvas is in a GtkCanvasViewport.
	*/
	return motion_notify_handler (ev);
}

/** Called to request a redraw of our canvas.
 *  @param area Area to redraw, in canvas coordinates.
 */
void
GtkCanvas::request_redraw (Rect const & area)
{
	queue_draw_area (floor (area.x0), floor (area.y0), ceil (area.x1) - floor (area.x0), ceil (area.y1) - floor (area.y0));
}

/** Called to request that we try to get a particular size for ourselves.
 *  @param size Size to request, in pixels.
 */
void
GtkCanvas::request_size (Duple size)
{
	Duple req = size;

	if (req.x > INT_MAX) {
		req.x = INT_MAX;
	}

	if (req.y > INT_MAX) {
		req.y = INT_MAX;
	}
	
	set_size_request (req.x, req.y);
}

/** `Grab' an item, so that all events are sent to that item until it is `ungrabbed'.
 *  This is typically used for dragging items around, so that they are grabbed during
 *  the drag.
 *  @param item Item to grab.
 */
void
GtkCanvas::grab (Item* item)
{
	/* XXX: should this be doing gdk_pointer_grab? */
	_grabbed_item = item;
}

/** `Ungrab' any item that was previously grabbed */
void
GtkCanvas::ungrab ()
{
	/* XXX: should this be doing gdk_pointer_ungrab? */
	_grabbed_item = 0;
}

/** Create a GtkCanvaSViewport.
 *  @param hadj Adjustment to use for horizontal scrolling.
 *  @param vadj Adjustment to use for vertica scrolling.
 */
GtkCanvasViewport::GtkCanvasViewport (Gtk::Adjustment& hadj, Gtk::Adjustment& vadj)
	: Viewport (hadj, vadj)
{
	add (_canvas);
}

/** Handler for when GTK asks us what minimum size we want.
 *  @param req Requsition to fill in.
 */
void
GtkCanvasViewport::on_size_request (Gtk::Requisition* req)
{
	req->width = 16;
	req->height = 16;
}

/** Convert window coordinates to canvas coordinates by taking into account
 *  where we are scrolled to.
 *  @param wx Window x.
 *  @param wy Window y.
 *  @param cx Filled in with canvas x.
 *  @param cy Filled in with canvas y.
 */
void
GtkCanvasViewport::window_to_canvas (int wx, int wy, Coord& cx, Coord& cy) const
{
	cx = wx + get_hadjustment()->get_value ();
	cy = wy + get_vadjustment()->get_value ();
}

/** @return The visible area of the canvas, in canvas coordinates */
Rect
GtkCanvasViewport::visible_area () const
{
	Distance const xo = get_hadjustment()->get_value ();
	Distance const yo = get_vadjustment()->get_value ();
	return Rect (xo, yo, xo + get_allocation().get_width (), yo + get_allocation().get_height ());
}
