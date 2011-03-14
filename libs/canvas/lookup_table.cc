#include "canvas/lookup_table.h"
#include "canvas/group.h"

using namespace std;
using namespace ArdourCanvas;

LookupTable::LookupTable (Group const & group, int items_per_cell)
	: _group (group)
	, _items_per_cell (items_per_cell)
{
	build ();
}

void
LookupTable::clear ()
{
	boost::multi_array<Cell, 2>::extent_gen extents;
	_cells.resize (extents[0][0]);
}

void
LookupTable::build ()
{
	list<Item*> const & items = _group.items ();
	
	if (items.empty ()) {
		return;
	}

	int const cells = items.size() / _items_per_cell;
	int const dim = max (1.0, sqrt (cells));

	boost::multi_array<Cell, 2>::extent_gen extents;
	_cells.resize (extents[dim][dim]);

	Rect const bbox = _group.bounding_box ();
	_cell_size.x = bbox.width() / dim;
	_cell_size.y = bbox.height() / dim;

	for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {
		int x0, y0, x1, y1;
		area_to_indices ((*i)->bounding_box (), x0, y0, x1, y1);

		assert (x0 <= dim);
		assert (y0 <= dim);
		assert (x1 <= dim);
		assert (y1 <= dim);
		
		for (int x = x0; x < x1; ++x) {
			for (int y = y0; y < y1; ++y) {
				_cells[x][y].push_back (*i);
			}
		}
	}
}

void
LookupTable::area_to_indices (Rect const & area, int& x0, int& y0, int& x1, int& y1) const
{
	x0 = area.x0 / _cell_size.x;
	y0 = area.y0 / _cell_size.y;
	x1 = ((area.x1 - COORD_EPSILON) / _cell_size.x) + 1;
	y1 = ((area.y1 - COORD_EPSILON) / _cell_size.y) + 1;
}
	

list<Item*>
LookupTable::get (Rect const & area)
{
	list<Item*> items;
	int x0, y0, x1, y1;
	area_to_indices (area, x0, y0, x1, y1);

	for (int x = x0; x < x1; ++x) {
		for (int y = y0; y < y1; ++y) {
			for (Cell::const_iterator i = _cells[x][y].begin(); i != _cells[x][y].end(); ++i) {
				items.push_back (*i);
			}
		}
	}

	items.sort ();
	items.unique ();

	return items;
}
