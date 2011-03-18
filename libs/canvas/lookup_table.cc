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

	_cell_size.x = bbox.get().width() / _dimension;
	_cell_size.y = bbox.get().height() / _dimension;
	_offset.x = bbox.get().x0;
	_offset.y = bbox.get().y0;

//	cout << "BUILD bbox=" << bbox.get() << ", cellsize=" << _cell_size << ", offset=" << _offset << ", dimension=" << _dimension << "\n";

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

		/* XXX */
		assert (x0 >= 0);
		assert (y0 >= 0);
		assert (x1 >= 0);
		assert (y1 >= 0);
		//assert (x0 <= _dimension);
		//assert (y0 <= _dimension);
		//assert (x1 <= _dimension);
		//assert (y1 <= _dimension);

		if (x0 > _dimension) {
			cout << "WARNING: item outside bbox by " << (item_bbox_in_group.x0 - bbox.get().x0) << "\n";
			x0 = _dimension;
		}
		if (x1 > _dimension) {
			cout << "WARNING: item outside bbox by " << (item_bbox_in_group.x1 - bbox.get().x1) << "\n";
			x1 = _dimension;
		}
		if (y0 > _dimension) {
			cout << "WARNING: item outside bbox by " << (item_bbox_in_group.y0 - bbox.get().y0) << "\n";
			y0 = _dimension;
		}
		if (y1 > _dimension) {
			cout << "WARNING: item outside bbox by " << (item_bbox_in_group.y1 - bbox.get().y1) << "\n";
			y1 = _dimension;
		}

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

	Rect const offset_area = area.translate (-_offset);

	x0 = floor (offset_area.x0 / _cell_size.x);
	y0 = floor (offset_area.y0 / _cell_size.y);
	x1 = ceil  (offset_area.x1 / _cell_size.x);
	y1 = ceil  (offset_area.y1 / _cell_size.y);
}

void
LookupTable::point_to_indices (Duple point, int& x, int& y) const
{
	if (_cell_size.x == 0 || _cell_size.y == 0) {
		x = y = 0;
		return;
	}

	Duple const offset_point = point - _offset;

	x = floor (offset_point.x / _cell_size.x);
	y = floor (offset_point.y / _cell_size.y);
}

list<Item*>
LookupTable::items_at_point (Duple point) const
{
	int x;
	int y;
	point_to_indices (point, x, y);

	Cell const & cell = _cells[x][y];
	list<Item*> items;
	for (Cell::const_iterator i = cell.begin(); i != cell.end(); ++i) {
		boost::optional<Rect> const item_bbox = (*i)->bounding_box ();
		if (item_bbox) {
			Rect parent_bbox = (*i)->item_to_parent (item_bbox.get ());
			if (parent_bbox.contains (point)) {
				items.push_back (*i);
			}
		}
	}

	return items;
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

