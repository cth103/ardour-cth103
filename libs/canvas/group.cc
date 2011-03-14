#include <iostream>
#include <cairomm/context.h>
#include "canvas/group.h"
#include "canvas/types.h"

using namespace std;
using namespace ArdourCanvas;

Group::Group ()
	: Item (0)
	, _lut (*this, 64)
{
}

Group::Group (Group* parent)
	: Item (parent)
	, _lut (*this, 64)
{
}

void
Group::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	context->save ();
	Rect const our_bbox = bounding_box ();
	context->translate (our_bbox.x0, our_bbox.y0);

	list<Item*> items = _lut.get (area);
	
	for (list<Item*>::const_iterator i = items.begin(); i != items.end(); ++i) {
		boost::optional<Rect> r = (*i)->bounding_box().intersection (area);
		if (r) {
			(*i)->render (*r, context);
		}
	}

	context->restore ();
}

Rect
Group::bounding_box () const
{
	if (_items.empty ()) {
		return Rect (_position.x, _position.y, 0, 0);
	}
	
	list<Item*>::const_iterator i = _items.begin();
	Rect bbox = (*i)->bounding_box ();
	++i;

	while (i != _items.end()) {
		bbox.extend ((*i)->bounding_box ());
		++i;
	}

	bbox.translate (-_position);
	return bbox;
}

void
Group::add (Item* i)
{
	_items.push_back (i);
	_lut.add ();
}

void
Group::remove (Item* i)
{
	_items.remove (i);
	_lut.remove (i);
}
