#include "pbd/compose.h"
#include "pbd/stacktrace.h"
#include "canvas/group.h"
#include "canvas/item.h"
#include "canvas/canvas.h"
#include "canvas/debug.h"

using namespace std;
using namespace PBD;
using namespace ArdourCanvas;

Item::Item (Canvas* canvas)
	: _canvas (canvas)
	, _parent (0)
{
	init ();
}

Item::Item (Group* parent)
	: _parent (parent)
{
	assert (parent);
	_canvas = _parent->canvas ();
	
	init ();
}

Item::Item (Group* parent, Duple position)
	: _parent (parent)
	, _position (position)
{
	assert (parent);
	_canvas = _parent->canvas ();
	
	init ();
}

void
Item::init ()
{
	_visible = true;
	_bounding_box_dirty = true;
	_watch = false;
	
	if (_parent) {
		_parent->add (this);
	}

	DEBUG_TRACE (DEBUG::CanvasItems, string_compose ("new canvas item %1\n", this));
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

/** Set the position of this item in the parent's coordinates */
void
Item::set_position (Duple p)
{
	boost::optional<Rect> bbox = bounding_box ();
	boost::optional<Rect> pre_change_parent_bounding_box;
	if (bbox) {
		pre_change_parent_bounding_box = item_to_parent (bbox.get());
	}
	
	_position = p;

	_canvas->item_moved (this, pre_change_parent_bounding_box);

	if (_parent) {
		_parent->child_changed ();
	}
}

void
Item::set_x_position (Coord x)
{
	set_position (Duple (x, _position.y));
}

void
Item::set_y_position (Coord y)
{
	set_position (Duple (_position.x, y));
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

Rect
Item::parent_to_item (Rect const & d) const
{
	return d.translate (- _position);
}

void
Item::unparent ()
{
	_canvas = 0;
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
	_canvas = _parent->canvas ();
	_parent->add (this);
}

void
Item::grab_focus ()
{
	/* XXX */
}

/** @return Bounding box in this item's coordinates */
boost::optional<Rect>
Item::bounding_box () const
{
	if (_bounding_box_dirty) {
		compute_bounding_box ();
	}

	assert (!_bounding_box_dirty);
	return _bounding_box;
}

/* XXX may be called even if bbox is not changing ... bit grotty */
void
Item::begin_change ()
{
	_pre_change_bounding_box = bounding_box ();
}

void
Item::end_change ()
{
	_canvas->item_changed (this, _pre_change_bounding_box);
	
	if (_parent) {
		_parent->child_changed ();
	}
}

void
Item::move (Duple movement)
{
	set_position (position() + movement);
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
