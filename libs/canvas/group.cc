#include <iostream>
#include <cairomm/context.h>
#include "canvas/group.h"
#include "canvas/types.h"

using namespace std;
using namespace ArdourCanvas;

Group::Group (Group* parent)
	: Item (parent)
{

}

void
Group::render (Rect const & area, Cairo::RefPtr<Cairo::Context> context) const
{
	context->save ();
	Rect const our_bbox = bounding_box ();
	context->translate (our_bbox.x0, our_bbox.y0);
	
	for (list<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		Rect const draw = (*i)->bounding_box().intersection (area);
		if (draw.width() > 0 && draw.height() > 0) {
			(*i)->render (draw, context);
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
}

void
Group::remove (Item* i)
{
	_items.remove (i);
}
