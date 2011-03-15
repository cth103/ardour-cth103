#include "canvas/lookup_table.h"
#include "canvas/group.h"

using namespace std;
using namespace ArdourCanvas;

LookupTable::LookupTable (Group const & group, int items_per_cell)
	: _group (group)
	, _items_per_cell (items_per_cell)
	, _added (false)
{
	build ();
}

void
LookupTable::build ()
{
	list<Item*> const & items = _group.items ();

	/* number of cells */
	int const cells = items.size() / _items_per_cell;
	/* hence number down each side of the table's square */
	_dimension = max (1, int (rint (sqrt (cells))));

	boost::multi_array<Cell, 2>::extent_gen extents;
	_cells.resize (extents[_dimension][_dimension]);

	/* our group's bounding box in its coordinates */
	boost::optional<Rect> bbox = _group.bounding_box ();
	if (!bbox) {
		return;
	}

	_cell_size.x = bbox.get().x1 / _dimension;
	_cell_size.y = bbox.get().y1 / _dimension;

	for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {

		/* item bbox in its own coordinates */
		boost::optional<Rect> item_bbox = (*i)->bounding_box ();
		if (!item_bbox) {
			continue;
		}

		/* and in the group's coordinates */
		Rect const item_bbox_in_group = (*i)->item_to_parent (item_bbox.get ());
		
		int x0, y0, x1, y1;
		area_to_indices (item_bbox_in_group, x0, y0, x1, y1);

		assert (x0 <= _dimension);
		assert (y0 <= _dimension);
		assert (x1 <= _dimension);
		assert (y1 <= _dimension);
		
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
	if (_cell_size.x == 0 || _cell_size.y == 0) {
		x0 = y0 = x1 = y1 = 0;
		return;
	}

	x0 = floor (area.x0 / _cell_size.x);
	y0 = floor (area.y0 / _cell_size.y);
	x1 = ceil (area.x1 / _cell_size.x);
	y1 = ceil (area.y1 / _cell_size.y);
}
	
/** @param area Area in our owning group's coordinates */
list<Item*>
LookupTable::get (Rect const & area)
{
	list<Item*> items;
	int x0, y0, x1, y1;
	area_to_indices (area, x0, y0, x1, y1);

	x0 = min (_dimension, x0);
	y0 = min (_dimension, y0);
	x1 = min (_dimension, x1);
	y1 = min (_dimension, y1);

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

void
LookupTable::remove (Item* i)
{
	for (int x = 0; x < _dimension; ++x) {
		for (int y = 0; y < _dimension; ++y) {
			_cells[x][y].remove (i);
		}
	}
}
