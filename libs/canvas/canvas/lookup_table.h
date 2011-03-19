#ifndef __CANVAS_LOOKUP_TABLE_H__
#define __CANVAS_LOOKUP_TABLE_H__

#include <list>
#include <boost/multi_array.hpp>
#include "canvas/types.h"

class LookupTableTest;

namespace ArdourCanvas {

class Item;
class Group;

class LookupTable
{
public:
	LookupTable (Group const &, int);
	std::list<Item*> get (Rect const &);
	std::list<Item*> items_at_point (Duple) const;

	static int default_items_per_cell;

private:

	void build ();
	void area_to_indices (Rect const &, int &, int &, int &, int &) const;
	void point_to_indices (Duple, int &, int &) const;

	friend class ::LookupTableTest;

	Group const & _group;
	typedef std::list<Item*> Cell;
	int _items_per_cell;
	int _dimension;
	Duple _cell_size;
	Duple _offset;
	boost::multi_array<Cell, 2> _cells;
	bool _added;
};

}

#endif
