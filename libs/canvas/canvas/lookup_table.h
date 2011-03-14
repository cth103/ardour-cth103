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

private:
	friend class ::LookupTableTest;
	
	typedef std::list<Item*> Cell;
	int _items_per_cell;
	Duple _cell_size;
	boost::multi_array<Cell, 2> _cells;
};


};
