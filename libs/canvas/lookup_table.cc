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
LookupTable::clear ()
{
	boost::multi_array<Cell, 2>::extent_gen extents;
	_cells.resize (extents[0][0]);
}

void
LookupTable::build ()
{
	list<Item*> const & items = _group.items ();

	int const cells = items.size() / _items_per_cell;
	_dimension = max (1, int (rint (sqrt (cells))));

	boost::multi_array<Cell, 2>::extent_gen extents;
	_cells.resize (extents[_dimension][_dimension]);

	Rect const bbox = _group.bounding_box ();
	_cell_size.x = bbox.width() / _dimension;
	_cell_size.y = bbox.height() / _dimension;

	for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {
		int x0, y0, x1, y1;
		area_to_indices ((*i)->bounding_box (), x0, y0, x1, y1);

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
	

list<Item*>
LookupTable::get (Rect const & area)
{
	if (_added) {
		if (_cell_size.x == 0 && _cell_size.y == 0) {
			build ();
		} else {
			bool build_needed = false;
			list<Item*> const & items = _group.items ();
			for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {
				int x0, y0, x1, y1;
				area_to_indices ((*i)->bounding_box (), x0, y0, x1, y1);
				if (x0 >= _dimension || x1 >= _dimension || y0 >= _dimension || y1 >= _dimension) {
					build_needed = true;
					break;
				}
			}
			
			if (build_needed) {
				clear ();
				build ();
			} else {
				for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {
					add_to_existing (*i);
				}
			}
		}
		
		_added = false;
	}

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
LookupTable::add ()
{
	_added = true;
}

void
LookupTable::add_to_existing (Item* i)
{
	int x0, y0, x1, y1;
	area_to_indices (i->bounding_box (), x0, y0, x1, y1);

	for (int x = x0; x < x1; ++x) {
		for (int y = y0; y < y1; ++y) {
			_cells[x][y].push_back (i);
		}
	}
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
