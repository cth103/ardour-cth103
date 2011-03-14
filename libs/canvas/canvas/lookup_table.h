#include <list>
#include <boost/multi_array.hpp>
#include "canvas/types.h"

class LookupTableTest;

namespace ArdourCanvas
{

class Item;
class Group;

class LookupTable
{
public:
	LookupTable (Group const &, int);
	std::list<Item*> get (Rect const &);

private:

	void area_to_indices (Rect const &, int&, int&, int&, int&) const;
	
	friend class ::LookupTableTest;
	
	typedef std::list<Item*> Cell;
	int _items_per_cell;
	Duple _cell_size;
	boost::multi_array<Cell, 2> _cells;
};


};
