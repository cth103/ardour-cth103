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

/** @file  canvas/item.h
 *  @brief Declaration of the item parent class.
 */

#ifndef __CANVAS_ITEM_H__
#define __CANVAS_ITEM_H__

#include <stdint.h>
#include <cairomm/context.h>
#include <gdk/gdkevents.h>
#include "pbd/signals.h"
#include "canvas/types.h"

class XMLNode;

namespace Canvas
{

class Canvas;
class Group;
class Rect;	

/** The parent class for anything that goes on the canvas.
 *
 *  Items have a position, which is expressed in the coordinates of the parent.
 *  They also have a bounding box, which describes the area in which they have
 *  drawable content, which is expressed in their own coordinates (whose origin
 *  is at the item position).
 *
 *  Any item that is being displayed on a canvas has a pointer to that canvas,
 *  and all except the `root group' have a pointer to their parent group.
 *
 *  An item can be transformed using one of its parent group's Transforms,
 *  in which case the item's coordinates are transformed before being plotted.
 */
	
class Item
{
public:
	Item (Canvas *);
	Item (Group *);
	Item (Group *, Duple);
	Item (Group *, TransformIndex);
	virtual ~Item ();

	/** Render this item to a Cairo context.
	 *  @param area Area to draw in this item's coordinates.
	 */
	virtual void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const = 0;

	/** Add the items at a given point to a vector of Items.  This is a stub; if it
	 *  gets called, we assume that the caller has already checked that this item
	 *  covers the point in question.
	 */
	virtual void add_items_at_point (Duple, std::vector<Item const *>& items) const {
		items.push_back (this);
	}

	/** Update _bbox and _bbox_dirty */
	virtual void compute_bbox () const = 0;

	/** Get the state of this item as XML.
	 *  @return State.
	 */
	virtual XMLNode* get_state () const = 0;

	/** Set the state of this item from some XML.
	 *  @param node XML Node.
	 */
	virtual void set_state (XMLNode const * node) = 0;

	void grab ();
	void ungrab ();
	
	void add_item_state (XMLNode *) const;
	void set_item_state (XMLNode const *);

	void unparent ();
	void reparent (Group *);

	/** @return Parent group, or 0 if this is the root group */
	Group* parent () const {
		return _parent;
	}

	void set_position (Duple);
	void set_x_position (Coord);
	void set_y_position (Coord);
	void move (Duple);

	/** @return Position of this item in the parent's coordinates */
	Duple position () const {
		return _position;
	}

	boost::optional<Rect> bbox () const;
	
	Duple item_to_parent (Duple const &) const;
	Rect item_to_parent (Rect const &) const;
	Duple parent_to_item (Duple const &) const;
	Rect parent_to_item (Rect const &) const;
	/* XXX: it's a pity these aren't the same form as item_to_parent etc.,
	   but it makes a bit of a mess in the rest of the code if they are not.
	*/
	void canvas_to_item (Coord &, Coord &) const;
	void item_to_canvas (Coord &, Coord &) const;
	Rect item_to_canvas (Rect const &) const;

	void raise_to_top ();
	void raise (int);
	void lower_to_bottom ();

	void hide ();
	void show ();

	/** @return true if this item is visible (ie it will be rendered),
	 *  otherwise false
	 */
	bool visible () const {
		return _visible;
	}

	/** @return Our canvas, or 0 if we are not attached to one */
	Canvas* canvas () const {
		return _canvas;
	}

	void set_ignore_events (bool);
	bool ignore_events () const {
		return _ignore_events;
	}

	void set_data (std::string const &, void *);
	void* get_data (std::string const &) const;

	/** @return The transform index of the parent's transform which is
	 *  applied to this item, or IDENTITY for the identity transform.
	 */
	TransformIndex transform_index () const {
		return _transform_index;
	}
	
	/* XXX: maybe this should be a PBD::Signal */

	/** An accumulator for the Event signal below.  Each handler that
	 *  is attached to the signal is called in turn, until one returns
	 *  true, at which point the calls stop.  The accumulator then returns
	 *  true if any handler handled the signal, or false if not.
	 */
	template <class T>
	struct EventAccumulator {
		typedef T result_type;
		template <class U>
		result_type operator () (U first, U last) {
			while (first != last) {
				if (*first) {
					/* this handler handled the signal, so we're done */
					return true;
				}
				++first;
			}
			return false;
		}
	};

	/** Emitted on any event that happens to this item */
	sigc::signal<bool, GdkEvent*>::accumulated<EventAccumulator<bool> > Event;

#ifdef CANVAS_DEBUG
	/** A name for this item (for debugging purposes) */
	std::string name;
#endif
	
#ifdef CANVAS_COMPATIBILITY
	void grab_focus ();
#endif	
	
	
protected:

	void begin_change ();
	void end_change ();

	/** our canvas */
	Canvas* _canvas;
	/** parent group; may be 0 if we are the root group or if we have been unparent()ed */
	Group* _parent;
	/** position of this item in parent coordinates */
	Duple _position;
	/** true if this item is visible (ie to be drawn), otherwise false */
	bool _visible;
	/** our bounding box before any change that is currently in progress, in the parent's coordinate */
	boost::optional<Rect> _pre_parent_bbox;

	/** our bounding box; may be out of date if _bbox_dirty is true */
	mutable boost::optional<Rect> _bbox;
	/** true if _bbox might be out of date, false if its definitely not */
	mutable bool _bbox_dirty;
	/** index of the parent's transform to apply to this item, or IDENTITY */
	TransformIndex _transform_index;

	/** Arbitrary map of data attached to this item.
	 *  XXX: this is a bit grubby.
	 */
	std::map<std::string, void *> _data;

private:
	void init ();

	/** true if this item should ignore all events, otherwise false */
	bool _ignore_events;
};

}

#endif
