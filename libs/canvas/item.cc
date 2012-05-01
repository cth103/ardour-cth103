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

/** @file  canvas/item.cc
 *  @brief Implementation of the item parent class.
 */

#include "pbd/compose.h"
#include "pbd/convert.h"
#include "pbd/stacktrace.h"
#include "pbd/xml++.h"
#include "ardour/utils.h"
#include "canvas/group.h"
#include "canvas/item.h"
#include "canvas/canvas.h"
#include "canvas/debug.h"

using namespace std;
using namespace PBD;
using namespace Canvas;

/** Construct an Item with no parent.
 *  @param canvas This item's canvas.
 */
Item::Item (Canvas* canvas)
	: _canvas (canvas)
	, _parent (0)
{
	_transform_index = -1;
	
	init ();
}

/** Construct an Item with a parent.
 *  @param parent This item's parent group.
 */
Item::Item (Group* parent)
	: _parent (parent)
{
	assert (parent);
	_canvas = _parent->canvas ();
	_transform_index = -1;
	
	init ();
}

/** Construct an Item, with a parent, which uses ones of its parents transforms.
 *  This means that the coordinates that the item uses will be transformed with
 *  a parent's transform.
 *  @param parent This item's parent group.
 *  @param transform_index Index of the parent transform to apply to this item.
 */
Item::Item (Group* parent, TransformIndex transform_index)
	: _parent (parent)
	, _transform_index (transform_index)
{
	assert (parent);
	_canvas = _parent->canvas ();
	
	init ();
}

/** Construct an Item, with a parent, at a given position.
 *  @param parent This item's parent group.
 *  @param position Position of the item within the parent, in the parent's coordinates.
 */
Item::Item (Group* parent, Duple position)
	: _parent (parent)
	, _position (position)
{
	assert (parent);
	_canvas = _parent->canvas ();
	_transform_index = -1;
	
	init ();
}

/** Common initialisation */
void
Item::init ()
{
	_visible = true;
	_bbox_dirty = true;
	_ignore_events = false;
	
	if (_parent) {
		_parent->add (this);
	}

	DEBUG_TRACE (DEBUG::CanvasItems, string_compose ("new canvas item %1\n", this));
}	

/** Item destructor */
Item::~Item ()
{
	_canvas->item_going_away (this, _bbox);
	
	if (_parent) {
		_parent->remove (this);
	}
}

/** Set the position of this item in the parent's coordinates.
 *  @param p New position.
 */
void
Item::set_position (Duple p)
{
	if (_position == p) {
		return;
	}
	
	begin_change ();
	
	_position = p;

	end_change ();
}

/** Set the x position of this item in the parent's coordinates.
 *  @param x New x position.
 */
void
Item::set_x_position (Coord x)
{
	set_position (Duple (x, _position.y));
}

/** Set the y position of this item in the parent's coordinates.
 *  @param y New y position.
 */
void
Item::set_y_position (Coord y)
{
	set_position (Duple (_position.x, y));
}

/** Raise this item to the top of its parent's stack */
void
Item::raise_to_top ()
{
	assert (_parent);
	_parent->raise_child_to_top (this);
}

/** Raise this item by a given number of levels in its parent's stack.
 *  @param levels Number of levels to raise by.
 */
void
Item::raise (int levels)
{
	assert (_parent);
	_parent->raise_child (this, levels);
}

/** Raise this item to the bottom of its parent's stack */
void
Item::lower_to_bottom ()
{
	assert (_parent);
	_parent->lower_child_to_bottom (this);
}

/** Hide this item */
void
Item::hide ()
{
	if (!_visible) {
		return;
	}
	
	begin_change ();
	
	_visible = false;

	end_change ();
}

/** Show this item */
void
Item::show ()
{
	if (_visible) {
		return;
	}
	
	begin_change ();
	
	_visible = true;

	end_change ();
}

/** Convert a Duple from this item's coordinates to that of its parent.
 *  @param d Duple to convert.
 *  @return Converted Duple.
 */
Duple
Item::item_to_parent (Duple const & d) const
{
	if (_transform_index == IDENTITY) {
		return d.translate (_position);
	}

	Transform const & transform = _parent->transform (_transform_index);
	return d.scale(transform.scale).translate (_position + transform.translation);
}

/** Convert a Rect from this item's coordinates to that of its parent.
 *  @param r Rect to convert.
 *  @return Converted Rect.
 */
Rect
Item::item_to_parent (Rect const & r) const
{
	if (_transform_index == IDENTITY) {
		return r.translate (_position);
	}

	Transform const & transform = _parent->transform (_transform_index);
	return r.scale(transform.scale).translate (_position + transform.translation);
}

/** Convert a Duple from this item's parent's coordinates to that of the item.
 *  @param d Duple to convert.
 *  @return Converted Duple.
 */
Duple
Item::parent_to_item (Duple const & d) const
{
	if (_transform_index == IDENTITY) {
		return d.translate (- _position);
	}

	Transform const & transform = _parent->transform (_transform_index);
	return d.translate (- _position - transform.translation).scale (Duple (1, 1) / transform.scale);
}

/** Convert a Rect from this item's parent's coordinates to that of the item.
 *  @param r Rect to convert.
 *  @return Converted Rect.
 */
Rect
Item::parent_to_item (Rect const & r) const
{
	if (_transform_index == IDENTITY) {
		return r.translate (- _position);
	}

	Transform const & transform = _parent->transform (_transform_index);
	return r.translate (- _position - transform.translation).scale (Duple (1, 1) / transform.scale);
}

/** Unparent this item from its parent group and its canvas */
void
Item::unparent ()
{
	_canvas = 0;
	_parent = 0;
}

/** Reparent this item to a new group.
 *  @param new_parent New parent group.
 */
void
Item::reparent (Group* new_parent)
{
	if (_parent) {
		_parent->remove (this);
	}

	assert (new_parent);

	_parent = new_parent;
	_canvas = _parent->canvas ();
	_parent->add (this);
}

void
Item::grab_focus ()
{
	/* XXX */
}

/** @return Bounding box in this item's coordinates */
boost::optional<Rect>
Item::bbox () const
{
	if (_bbox_dirty) {
		compute_bbox ();
	}

	assert (!_bbox_dirty);
	return _bbox;
}

/** Called when a change is about to be made to this item that will
 *  require some kind of redraw.
 */
void
Item::begin_change ()
{
	/* Just make a note of our bounding box before the change */
	if (bbox ()) {
		_pre_parent_bbox = item_to_parent (bbox().get ());
	} else {
		_pre_parent_bbox.reset ();
	}
}

/** Called when a change has been made to this item that will
 *  require some kind of redraw.  Should follow a call to begin_change ().
 */
void
Item::end_change ()
{
	_canvas->item_changed (this, _pre_parent_bbox);
	
	if (_parent) {
		_parent->child_changed ();
	}
}

/** Move this item's position by a given amount.
 *  @param movement Movement to apply.
 */
void
Item::move (Duple movement)
{
	set_position (position() + movement);
}

/** Add this item's state to an XML node.
 *  @param node Node to add state to.
 */
void
Item::add_item_state (XMLNode* node) const
{
	node->add_property ("x-position", string_compose ("%1", _position.x));
	node->add_property ("y-position", string_compose ("%1", _position.y));
	node->add_property ("visible", _visible ? "yes" : "no");
}

/** Set this item's state from an XML node.
 *  @param node Node to read state from.
 */
void
Item::set_item_state (XMLNode const * node)
{
	_position.x = atof (node->property("x-position")->value().c_str());
	_position.y = atof (node->property("y-position")->value().c_str());
	_visible = string_is_affirmative (node->property("visible")->value());
}

/** Grab this item, so that until it is ungrab()ed all events will
 *  be delivered to it alone.
 */
void
Item::grab ()
{
	assert (_canvas);
	_canvas->grab (this);
}

/** Reverse the effects of grab () */
void
Item::ungrab ()
{
	assert (_canvas);
	_canvas->ungrab ();
}

/** Set some arbitrary data to be stored in this item with a given key.
 *  The data can be retrieved using get_data ().
 *  @param key Key.
 *  @param data Data.
 */
void
Item::set_data (string const & key, void* data)
{
	_data[key] = data;
}

/** Retrieve a piece of data attached to this item with set_data ().
 *  @param key Key.
 *  @return Data.
 */
void *
Item::get_data (string const & key) const
{
	map<string, void*>::const_iterator i = _data.find (key);
	if (i == _data.end ()) {
		return 0;
	}
	
	return i->second;
}

/** Convert a pair of coordinates from this item's coordinates to
 *  that of the canvas.
 *  @param x x coordinate to convert; filled in with the converted coordinate.
 *  @param y y coordinate to convert; filled in with the converted coordinate.
 */
void
Item::item_to_canvas (Coord& x, Coord& y) const
{
	Duple d (x, y);
	Item const * i = this;

	/* work our way up the tree until we hit the root */
	while (i) {
		d = i->item_to_parent (d);
		i = i->_parent;
	}
		
	x = d.x;
	y = d.y;
}

/** Convert a pair of coordinates from the canvas to this item's coordinates.
 *  @param x x coordinate to convert; filled in with the converted coordinate.
 *  @param y y coordinate to convert; filled in with the converted coordinate.
 */
void
Item::canvas_to_item (Coord& x, Coord& y) const
{
	Duple d (x, y);
	Item const * i = this;

	/* work our way up the tree until we hit the root */
	while (i) {
		d = i->parent_to_item (d);
		i = i->_parent;
	}

	x = d.x;
	y = d.y;
}

/** Convert a Rect from this item's coordinates to that of the canvas.
 *  @param area Rect to convert.
 *  @return Converted Rect.
 */
Rect
Item::item_to_canvas (Rect const & area) const
{
	Rect r = area;
	Item const * i = this;

	while (i) {
		r = i->item_to_parent (r);
		i = i->parent ();
	}

	return r;
}

/** Set this item to ignore all events, so that it will always pass on
 *  events to the item below it.
 */
void
Item::set_ignore_events (bool ignore)
{
	_ignore_events = ignore;
}
