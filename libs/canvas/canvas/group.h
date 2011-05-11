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

/** @file  canvas/group.h
 *  @brief Declaration for a group of items.
 */

#ifndef __CANVAS_GROUP_H__
#define __CANVAS_GROUP_H__

#include <list>
#include <vector>
#include "canvas/item.h"
#include "canvas/types.h"

namespace ArdourCanvas {

/** An Item which is itself a group of other items */
class Group : public Item
{
public:
	explicit Group (Group *);
	explicit Group (Group *, Duple);
	~Group ();

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	virtual void compute_bbox () const;
	XMLNode* get_state () const;
	void set_state (XMLNode const *);

	void add (Item *);
	void remove (Item *);

	/** @return Our items, in order from back to front */
	std::vector<Item*> const & items () const {
		return _items;
	}

	void raise_child_to_top (Item *);
	void raise_child (Item *, int);
	void lower_child_to_bottom (Item *);
	void child_changed ();

	TransformIndex add_transform (Transform const &);
	void set_transform (TransformIndex, Transform const &);
	Transform const & transform (TransformIndex) const;

	void add_items_at_point (Duple, std::vector<Item const *> &) const;

protected:
	
	explicit Group (Canvas *);
	
private:
	Group (Group const &);

	/** our items, from lowest to highest in the stack */
	std::vector<Item*> _items;

	/** our transforms */
	std::vector<Transform> _transforms;
};

}

#endif
