#include "canvas/group.h"
#include "canvas/item.h"

using namespace std;
using namespace ArdourCanvas;

Item::Item (Group* parent)
	: _parent (parent)
{
	if (_parent) {
		_parent->add (this);
	}
}

Item::Item (Group* parent, Duple position)
	: _parent (parent)
	, _position (position)
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

void
Item::raise_to_top ()
{
	assert (_parent);
	_parent->raise_child_to_top (this);
}

void
Item::raise (int levels)
{
	assert (_parent);
	_parent->raise_child (this, levels);
}

void
Item::lower_to_bottom ()
{
	assert (_parent);
	_parent->lower_child_to_bottom (this);
}

void
Item::hide ()
{
	_visible = false;
}

void
Item::show ()
{
	_visible = true;
}

Duple
Item::item_to_parent (Duple const & d) const
{
	return d.translate (_position);
}

Duple
Item::parent_to_item (Duple const & d) const
{
	return d.translate (- _position);
}

void
Item::unparent ()
{
	_parent = 0;
}

void
Item::reparent (Group* new_parent)
{
	if (_parent) {
		_parent->remove (this);
	}

	assert (new_parent);

	_parent = new_parent;
	_parent->add (this);
}

void
Item::grab_focus ()
{
	/* XXX */
}

#ifdef CANVAS_COMPATIBILITY
void
Item::set_data (char const * key, void* data)
{
	_data[key] = data;
}

void*
Item::get_data (char const * key)
{
	return _data[key];
}

void
Item::i2w (double& x, double& y)
{
	Duple d (x, y);
	Item* i = this;
	
	while (i) {
		d = i->item_to_parent (d);
		i = i->_parent;
	}
		
	x = d.x;
	y = d.y;
}

void
Item::w2i (double& x, double& y)
{
	Duple d (x, y);
	Item* i = this;

	while (i) {
		d = i->parent_to_item (d);
		i = i->_parent;
	}

	x = d.x;
	y = d.y;
}

void
Item::get_bounds (double& x0, double& y0, double& x1, double& y1) const
{
	boost::optional<Rect> const bbox = bounding_box ();
	if (!bbox) {
		return;
	}

	x0 = bbox.get().x0;
	y0 = bbox.get().y0;
	x1 = bbox.get().x1;
	y1 = bbox.get().y1;
}

void
Item::move (double dx, double dy)
{
	_position.x += dx;
	_position.y += dy;
}

void
Item::grab (int, Gdk::Cursor, uint32_t &)
{
	/* XXX */
}

void
Item::grab (int, uint32_t)
{
	/* XXX */
}

void
Item::ungrab (int)
{
	/* XXX */
}

#endif
