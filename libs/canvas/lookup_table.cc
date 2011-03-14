#include "canvas/lookup_table.h"
#include "canvas/group.h"

using namespace std;
using namespace ArdourCanvas;

LookupTable::LookupTable (Group const & group, int items_per_cell)
	: _items_per_cell (items_per_cell)
{
	list<Item*> const & items = group.items ();
	
	if (items.empty ()) {
		return;
	}

	int const cells = items.size() / _items_per_cell;
	int const dim = max (1.0, sqrt (cells));

	boost::multi_array<Cell, 2>::extent_gen extents;
	_cells.resize (extents[dim][dim]);

	Rect const bbox = group.bounding_box ();
	_cell_size.x = bbox.width() / dim;
	_cell_size.y = bbox.height() / dim;

	for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {
		Rect const item_bbox = (*i)->bounding_box ();
		int const x0 = item_bbox.x0 / _cell_size.x;
		int const y0 = item_bbox.y0 / _cell_size.y;
		int const x1 = ((item_bbox.x1 - COORD_EPSILON) / _cell_size.x) + 1;
		int const y1 = ((item_bbox.y1 - COORD_EPSILON) / _cell_size.y) + 1;

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
