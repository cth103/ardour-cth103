#ifndef __CANVAS_GROUP_H__
#define __CANVAS_GROUP_H__

#include <list>
#include "canvas/item.h"
#include "canvas/types.h"
#include "canvas/lookup_table.h"

namespace ArdourCanvas {

class Group : public Item
{
public:
	Group ();
	explicit Group (Group *);
	explicit Group (Group *, Duple);
	~Group ();

	void add (Item *);
	void remove (Item *);
	std::list<Item*> const & items () const {
		return _items;
	}

	void raise_child_to_top (Item *);
	void raise_child (Item *, int);
	void lower_child_to_bottom (Item *);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	boost::optional<Rect> bounding_box () const;

#ifdef CANVAS_COMPATIBILITY
	Coord& property_x () {
		return _position.x;
	}

	Coord& property_y () {
		return _position.y;
	}
#endif	
	
private:
	Group (Group const &);
	void ensure_lut () const;
	void invalidate_lut () const;

	/* our items, from lowest to highest in the stack */
	std::list<Item*> _items;

	mutable LookupTable* _lut;
};

}

#endif
