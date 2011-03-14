#include "canvas/group.h"
#include "canvas/item.h"

using namespace ArdourCanvas;

Item::Item (Group* parent)
	: _parent (parent)
{
	if (_parent) {
		_parent->add (this);
	}
}

Item::~Item ()
{
	if (_parent) {
		_parent->remove (this);
	}
}

Rect
Item::item_to_parent (Rect const & r) const
{
	return r.translate (_position);
}

void
Item::set_position (Duple p)
{
	_position = p;
}
