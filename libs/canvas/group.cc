#include <iostream>
#include <cairomm/context.h>
#include "canvas/group.h"
#include "canvas/types.h"

using namespace std;
using namespace ArdourCanvas;

Group::Group ()
	: Item (0)
	, _lut (0)
{
	
}

Group::Group (Group* parent)
	: Item (parent)
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
	
	for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {
		if (!(*i)->visible ()) {
			continue;
		}
		
		boost::optional<Rect> item_bbox = (*i)->bounding_box ();
		if (!item_bbox) {
			continue;
		}
		
		Rect const group_bbox = (*i)->item_to_parent (*item_bbox);
		boost::optional<Rect> r = group_bbox.intersection (area);
		if (r) {
			context->save ();
			context->translate ((*i)->position().x, (*i)->position().y);
			Rect sub_area = *r;
			sub_area.translate ((*i)->position ());
			(*i)->render (sub_area, context);
			context->restore ();
		}
	}
}

boost::optional<Rect>
Group::bounding_box () const
{
	if (_items.empty ()) {
		return boost::optional<Rect> ();
	}

	bool have_initial = false;
	Rect bbox;
	
	for (list<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		boost::optional<Rect> item_bbox = (*i)->bounding_box ();
		if (!item_bbox) {
			continue;
		}

		Rect group_bbox = item_bbox.get().translate ((*i)->position ());
		if (have_initial) {
			bbox = bbox.extend (group_bbox);
		} else {
			bbox = group_bbox;
			have_initial = true;
		}
	}

	return bbox;
}

void
Group::add (Item* i)
{
	_items.push_back (i);
	invalidate_lut ();
}

void
Group::remove (Item* i)
{
	_items.remove (i);
	if (_lut) {
		_lut->remove (i);
	}
}

void
Group::raise_to_top (Item* i)
{
	_items.remove (i);
	_items.push_back (i);
	invalidate_lut ();
}

void
Group::raise (Item* i, int levels)
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
Group::lower_to_bottom (Item* i)
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
		
