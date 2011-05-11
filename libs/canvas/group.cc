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

/** @file  canvas/group.cc
 *  @brief Implementation for a group of items.
 */

#include <iostream>
#include <cairomm/context.h>
#include "pbd/stacktrace.h"
#include "pbd/xml++.h"
#include "canvas/group.h"
#include "canvas/types.h"
#include "canvas/debug.h"
#include "canvas/item_factory.h"

using namespace std;
using namespace Canvas;

/** Construct a Group with no parent.
 *  @param canvas This group's canvas.
 */
Group::Group (Canvas* canvas)
	: Item (canvas)
{
	
}

/** Construct a Group.
 *  @param parent This group's parent group.
 */
Group::Group (Group* parent)
	: Item (parent)
{

}

/** Construct a Group, with a parent, at a given position.
 *  @param parent This group's parent group.
 *  @param position Position of the item group within the parent, in the parent's coordinates.
 */
Group::Group (Group* parent, Duple position)
	: Item (parent, position)
{

}

/** Group destructor */
Group::~Group ()
{
	for (vector<Item*>::iterator i = _items.begin(); i != _items.end(); ++i) {
		(*i)->unparent ();
	}

	_items.clear ();
}

void
Group::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	for (vector<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {

		if (!(*i)->visible ()) {
			continue;
		}
		
		boost::optional<Rect> item_bbox = (*i)->bbox ();
		if (!item_bbox) {
			continue;
		}

		/* convert the render area to our child's coordinates */
		Rect const item_area = (*i)->parent_to_item (area);

		/* intersect the child's render area with the child's bounding box */
		boost::optional<Rect> r = item_bbox.get().intersection (item_area);

		if (r) {
			/* render the intersection */
			context->save ();

			if ((*i)->transform_index() != IDENTITY) {
				Transform const & t = transform ((*i)->transform_index ());
				context->scale (t.scale.x, t.scale.y);
				context->translate (t.translation.x, t.translation.y);
			}

			context->translate ((*i)->position().x, (*i)->position().y);
			(*i)->render (r.get(), context);
			context->restore ();
		}
	}
}

void
Group::compute_bbox () const
{
	Rect bbox;
	bool have_one = false;

	for (vector<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {

		boost::optional<Rect> item_bbox = (*i)->bbox ();
		if (!item_bbox) {
			continue;
		}

		Rect group_bbox = (*i)->item_to_parent (item_bbox.get ());

		if (have_one) {
			bbox = bbox.extend (group_bbox);
		} else {
			bbox = group_bbox;
			have_one = true;
		}
	}

	if (!have_one) {
		_bbox = boost::optional<Rect> ();
	} else {
		_bbox = bbox;
	}

	_bbox_dirty = false;
}

/** Add an item to the top of this Group's stack.
 *  @param i Item to add.
 */
void
Group::add (Item* i)
{
	_items.push_back (i);
	_bbox_dirty = true;
	
	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: group add\n");
}

/** Remove an item from this Group's stack.
 *  @param i Item to remove.
 */
void
Group::remove (Item* i)
{
	vector<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());
	_items.erase (j);
	
	_bbox_dirty = true;
	
	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: group remove\n");
}

/** Raise one of this Group's items to the top of the stack.
 *  @param i Item to raise; must be in the Group.
 */
void
Group::raise_child_to_top (Item* i)
{
	vector<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());
	_items.erase (j);

	_items.push_back (i);
}

/** Raise one of this Group's items up a certain number of levels in the stack.
 *  @param i Item to raise; must be in the Group.
 *  @param levels Number of levels by which to raise the item.
 */
void
Group::raise_child (Item* i, int levels)
{
	vector<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());

	++j;

	while (levels > 0 && j != _items.end ()) {
		++j;
		--levels;
	}

	_items.insert (j, i);

	j = find (_items.begin(), _items.end(), i);
	_items.erase (j);
}

/** Lower one of this Group's items to the bottom of the stack.
 *  @param i Item to lower; must be in the Group.
 */
void
Group::lower_child_to_bottom (Item* i)
{
	vector<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());
	_items.erase (j);

	_items.insert (_items.begin (), i);
}

/** Called when one of our children has changed in some way */
void
Group::child_changed ()
{
	_bbox_dirty = true;

	if (_parent) {
		_parent->child_changed ();
	}
}

/** Find the items in this Group which contain a certain point, and add them to a vector.
 *  @param point Point to examine, in this Group's coordinates.
 *  @param items Vector to add items to.
 */
void
Group::add_items_at_point (Duple const point, vector<Item const *>& items) const
{
	boost::optional<Rect> const group_bbox = bbox ();
	if (!group_bbox || !group_bbox.get().contains (point)) {
		return;
	}

	/* This group contains the point, so add it to the list */
	Item::add_items_at_point (point, items);

	vector<Item*> our_items;

	/* Find those items in our stack which contain the point */
	for (vector<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		boost::optional<Rect> item_bbox = (*i)->bbox ();
		if (item_bbox) {
			Rect parent_bbox = (*i)->item_to_parent (item_bbox.get ());
			if (parent_bbox.contains (point)) {
				our_items.push_back (*i);
			}
		}
	}

	/* Call down recursively to check those items */
	for (vector<Item*>::iterator i = our_items.begin(); i != our_items.end(); ++i) {
		/* XXX: shouldn't this be parent_to_item? */
		(*i)->add_items_at_point (point - (*i)->position(), items);
	}
}

XMLNode *
Group::get_state () const
{
	XMLNode* node = new XMLNode ("Group");
	for (vector<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		node->add_child_nocopy (*(*i)->get_state ());
	}

	add_item_state (node);
	return node;
}

void
Group::set_state (XMLNode const * node)
{
	set_item_state (node);

	XMLNodeList const & children = node->children ();
	for (XMLNodeList::const_iterator i = children.begin(); i != children.end(); ++i) {
		/* this will create the item and add it to this group */
		create_item (this, *i);
	}
}

/** Add a transform to this Group.  This will not affect the group itself, but
 *  can subsequently be applied to any of the Group's direct children.
 *  @param transform Transform to add.
 *  @return Index of this transform for future reference.
 */
TransformIndex
Group::add_transform (Transform const & transform)
{
	_transforms.push_back (transform);
	return _transforms.size() - 1;
}

/** Set the details of a previously added transform.  This will effectively
 *  change a transform that was previously added, updating any children that
 *  are using it.
 *  @param index Index of the transform to change.
 *  @param transform New value of the transform.
 */
void
Group::set_transform (TransformIndex index, Transform const & transform)
{
	assert (index >= 0 && index < int (_transforms.size()));

	begin_change ();
	
	_transforms[index] = transform;
	_bbox_dirty = true;

	end_change ();
}

/** @param index Transform index.
 *  @return The transform.
 */
Transform const &
Group::transform (TransformIndex index) const
{
	assert (index >= 0 && index < int (_transforms.size()));
	
	return _transforms[index];
}
