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

/** @file  canvas/canvas.h
 *  @brief Declaration of the main canvas classes.
 */

#ifndef __CANVAS_CANVAS_H__
#define __CANVAS_CANVAS_H__

#include <gdkmm/window.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/viewport.h>
#include <cairomm/surface.h>
#include <cairomm/context.h>
#include "pbd/signals.h"
#include "canvas/root_group.h"

class XMLTree;

namespace ArdourCanvas
{

class Rect;
class Group;

/** A square buffer of pixels that is used as a cache for the contents
 *  of the canvas.  Implemented as just a Cairo ImageSurface which knows
 *  when it needs to be updated, and can ask the Canvas to draw onto it.
 */
class Tile
{
public:
	Tile (Canvas const *, int, int, int);

	void render ();

	/** @return our surface */
	Cairo::RefPtr<Cairo::Surface> surface () {
		return _surface;
	}

	/** @return true if this tile needs redrawing,
	 *  otherwise false.
	 */
	bool dirty () const {
		return _dirty;
	}

	void set_dirty ();
	
private:
	/** the canvas that we are being used for */
	Canvas const * _canvas;
	/** x index of the tile within the canvas */
	int _tx;
	/** y index of the tile within the canvas */
	int _ty;
	/** true if this tile is in need of a repaint */
	bool _dirty;

	/** our surface */
	Cairo::RefPtr<Cairo::ImageSurface> _surface;
	/** context for our surface */
	Cairo::RefPtr<Cairo::Context> _context;
};

/** The base class for our different types of canvas.
 *
 *  A canvas is an area which holds a collection of canvas items, which in
 *  turn represent shapes, text, etc.
 *
 *  The canvas has an arbitrarily large area, and is addressed in coordinates
 *  of screen pixels, with an origin of (0, 0) at the top left.  x increases
 *  rightwards and y increases downwards.
 *
 *  The canvas redraw speed is improved by dividing it up into `tiles', which
 *  are bitmapped squares.  These are blitted to the screen on expose events,
 *  and created/drawn as required.
 */
	
class Canvas
{
public:
	Canvas ();
	Canvas (XMLTree const *);
	virtual ~Canvas () {}

	/** called to request a redraw of an area of the canvas */
	virtual void request_redraw (Rect const &) = 0;
	/** called to ask the canvas to request a particular size from its host */
	virtual void request_size (Duple) = 0;
	/** called to ask the canvas' host to `grab' an item */
	virtual void grab (Item *) = 0;
	/** called to ask the canvas' host to `ungrab' any grabbed item */
	virtual void ungrab () = 0;

	void paint_from_tiles (Rect const &, Cairo::RefPtr<Cairo::Context> const &) const;
	void render_to_tile (Cairo::RefPtr<Cairo::Context>, int, int) const;

	/** @return root group */
	Group* root () {
		return &_root;
	}

	/** Called when an item is being destroyed */
	virtual void item_going_away (Item *, boost::optional<Rect>) {}
	void item_changed (Item *, boost::optional<Rect>);

	XMLTree* get_state () const;

	virtual Cairo::RefPtr<Cairo::Context> context () = 0;

	void suspend_updates ();
	void resume_updates ();

#ifdef CANVAS_DEBUG	
	/** @return a list of rectangles which have been rendered since
	 *  the canvas was created (for profiling / debugging purposes).
	 */
	std::list<Rect> const & renders () const {
		return _renders;
	}

	/** Set whether or not this canvas logs the renders that are
	 *  requested of it.
	 *  @param log true to log, otherwise false.
	 */
	void set_log_renders (bool log) {
		_log_renders = log;
	}

	mutable int tile_render_count;

#endif	

protected:
	void mark_item_area_dirty (Item *, Rect);
	void area_to_tiles (Rect const &, int &, int &, int &, int &) const;
	
	/** our root group */
	RootGroup _root;

	mutable std::vector<std::vector<boost::shared_ptr<Tile> > > _tiles;

	/** true if all updates are suspended, otherwise false */
	bool _updates_suspended;
	
#ifdef CANVAS_DEBUG	
	/** a list of rectangles which have been rendered since this canvas
	 *  was created (if _log_renders is true).
	 */
	mutable std::list<Rect> _renders;
	/** true to keep _renders up to date */
	bool _log_renders;
#endif

private:
	void ensure_tile (int, int) const;

	/** the width and height of the tiles used to cache our image */
	int _tile_size;
};

/** A Canvas which renders onto an in-memory pixbuf.  In Ardour's context,
 *  this is most useful for testing.
 */
class ImageCanvas : public Canvas
{
public:
	ImageCanvas (Duple size = Duple (1024, 1024));
	ImageCanvas (XMLTree const *, Duple size = Duple (1024, 1024));

	void request_redraw (Rect const &) {
		/* XXX */
	}

	void request_size (Duple) {
		/* XXX */
	}
	
	void grab (Item *) {}
	void ungrab () {}

	void render_to_image (Rect const &) const;
	void clear ();
	void write_to_png (std::string const &);

	Cairo::RefPtr<Cairo::Context> context ();

private:
	/** our Cairo surface */
	Cairo::RefPtr<Cairo::Surface> _surface;
	/** our Cairo context */
	Cairo::RefPtr<Cairo::Context> _context;
};

/** A canvas which renders onto a GTK EventBox */
class GtkCanvas : public Canvas, public Gtk::EventBox
{
public:
	GtkCanvas ();
	GtkCanvas (XMLTree const *);

	void request_redraw (Rect const &);
	void request_size (Duple);
	void grab (Item *);
	void ungrab ();

	Cairo::RefPtr<Cairo::Context> context ();
	
protected:
	bool on_expose_event (GdkEventExpose *);
	bool on_button_press_event (GdkEventButton *);
	bool on_button_release_event (GdkEventButton *);
	bool on_motion_notify_event (GdkEventMotion *);
	bool on_leave_notify_event (GdkEventCrossing *);
	
	bool button_handler (GdkEventButton *);
	bool motion_notify_handler (GdkEventMotion *);
	bool deliver_event (Duple, GdkEvent *);

private:
	void item_going_away (Item *, boost::optional<Rect>);
	bool send_leave_event (Item const *, double, double) const;

	/** the items that the mouse is currently over */
	std::vector<Item const *> _current_items;
	/** the item that is currently grabbed, or 0 */
	Item const * _grabbed_item;
};

/** A GTK::Viewport with a GtkCanvas inside it.  This provides a GtkCanvas
 *  that can be scrolled.
 */
class GtkCanvasViewport : public Gtk::Viewport
{
public:
	GtkCanvasViewport (Gtk::Adjustment &, Gtk::Adjustment &);

	/** @return our GtkCanvas */
	GtkCanvas* canvas () {
		return &_canvas;
	}

	void window_to_canvas (int, int, Coord &, Coord &) const;
	Rect visible_area () const;

protected:
	void on_size_request (Gtk::Requisition *);

private:
	/** our GtkCanvas */
	GtkCanvas _canvas;
};

}

#endif
