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
