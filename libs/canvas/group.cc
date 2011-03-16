#include <iostream>
#include <cairomm/context.h>
#include "canvas/group.h"
#include "canvas/types.h"

using namespace std;
using namespace ArdourCanvas;

Group::Group (Canvas* canvas)
	: Item (canvas)
	, _lut (0)
{
	
}

Group::Group (Group* parent)
	: Item (parent)
	, _lut (0)
{
	
}

Group::Group (Group* parent, Duple position)
	: Item (parent, position)
	, _lut (0)
{
	
}

Group::~Group ()
{
	for (list<Item*>::iterator i = _items.begin(); i != _items.end(); ++i) {
		(*i)->unparent ();
	}

	_items.clear ();
}

/** @param area Area to draw in this group's coordinates.
 *  @param context Context, set up with its origin at this group's position.
 */
void
Group::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	ensure_lut ();
	list<Item*> items = _lut->get (area);

	bounding_box ();
	
	for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {
		if (!(*i)->visible ()) {
			continue;
		}
		
		boost::optional<Rect> item_bbox = (*i)->bounding_box ();
		if (!item_bbox) {
			continue;
		}

		/* convert the render area to our child's coordinates */
		Rect const item_area = (*i)->parent_to_item (area);

		/* intersect the child's render area with the child's bounding box */
		boost::optional<Rect> r = item_bbox.get().intersection (item_area);
		
		if (r) {
			/* render the intersection */
			context->save ();
			context->translate ((*i)->position().x, (*i)->position().y);
			(*i)->render (r.get(), context);
			context->restore ();
		}
			
	}
}

void
Group::compute_bounding_box () const
{
	Rect bbox;
	bool have_one = false;

	for (list<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		boost::optional<Rect> item_bbox = (*i)->bounding_box ();
		if (!item_bbox) {
			continue;
		}

		Rect group_bbox = (*i)->item_to_parent (item_bbox.get ());
		if (have_one) {
			bbox = bbox.extend (group_bbox);
		} else {
			bbox = group_bbox;
			have_one = true;
		}
	}

	if (!have_one) {
		_bounding_box = boost::optional<Rect> ();
	} else {
		_bounding_box = bbox;
	}

	_bounding_box_dirty = false;
}

void
Group::add (Item* i)
{
	_items.push_back (i);
	invalidate_lut ();
	_bounding_box_dirty = true;
}

void
Group::remove (Item* i)
{
	_items.remove (i);
	invalidate_lut ();
	_bounding_box_dirty = true;
}

void
Group::raise_child_to_top (Item* i)
{
	_items.remove (i);
	_items.push_back (i);
	invalidate_lut ();
}

void
Group::raise_child (Item* i, int levels)
{
	list<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());

	++j;
	_items.remove (i);

	while (levels > 0 && j != _items.end ()) {
		++j;
		--levels;
	}

	_items.insert (j, i);
	invalidate_lut ();
}

void
Group::lower_child_to_bottom (Item* i)
{
	_items.remove (i);
	_items.push_front (i);
	invalidate_lut ();
}

void
Group::ensure_lut () const
{
	if (!_lut) {
		_lut = new LookupTable (*this, 64);
	}
}

void
Group::invalidate_lut () const
{
	delete _lut;
	_lut = 0;
}

void
Group::child_changed ()
{
	invalidate_lut ();
	_bounding_box_dirty = true;

	if (_parent) {
		_parent->child_changed ();
	}
}
