#ifndef __CANVAS_LOOKUP_TABLE_H__
#define __CANVAS_LOOKUP_TABLE_H__

#include <vector>
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
	~LookupTable ();
	std::vector<Item*> get (Rect const &);
	std::vector<Item*> items_at_point (Duple) const;

	static int default_items_per_cell;

private:

	void area_to_indices (Rect const &, int &, int &, int &, int &) const;
	void point_to_indices (Duple, int &, int &) const;

	friend class ::LookupTableTest;

	Group const & _group;
	typedef std::vector<Item*> Cell;
	int _items_per_cell;
	int _dimension;
	Duple _cell_size;
	Duple _offset;
	Cell** _cells;
	bool _added;
};

}

#endif
