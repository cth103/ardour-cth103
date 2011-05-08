#include "pbd/compose.h"
#include "pbd/stacktrace.h"
#include "pbd/xml++.h"
#include "ardour/utils.h"
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
	_transform_index = -1;
	
	init ();
}

Item::Item (Group* parent)
	: _parent (parent)
{
	assert (parent);
	_canvas = _parent->canvas ();
	_transform_index = -1;
	
	init ();
}

Item::Item (Group* parent, TransformIndex transform_index)
	: _parent (parent)
	, _transform_index (transform_index)
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
	_transform_index = -1;
	
	init ();
}

void
Item::init ()
{
	_visible = true;
	_bbox_dirty = true;
	_ignore_events = false;
	
	if (_parent) {
		_parent->add (this);
	}

	DEBUG_TRACE (DEBUG::CanvasItems, string_compose ("new canvas item %1\n", this));
}	

Item::~Item ()
{
	_canvas->item_going_away (this, _bbox);
	
	if (_parent) {
		_parent->remove (this);
	}
}

/** Set the position of this item in the parent's coordinates */
void
Item::set_position (Duple p)
{
	if (_position == p) {
		return;
	}
	
	begin_change ();
	
	_position = p;

	end_change ();
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
	if (!_visible) {
		return;
	}
	
	begin_change ();
	
	_visible = false;

	end_change ();
}

void
Item::show ()
{
	if (_visible) {
		return;
	}
	
	begin_change ();
	
	_visible = true;

	end_change ();
}

Duple
Item::item_to_parent (Duple const & d) const
{
	if (_transform_index == IDENTITY) {
		return d.translate (_position);
	}

	Transform const & transform = _parent->transform (_transform_index);
	return d.scale(transform.scale).translate (_position + transform.translation);
}

Rect
Item::item_to_parent (Rect const & r) const
{
	if (_transform_index == IDENTITY) {
		return r.translate (_position);
	}

	Transform const & transform = _parent->transform (_transform_index);
	return r.scale(transform.scale).translate (_position + transform.translation);
}

Duple
Item::parent_to_item (Duple const & d) const
{
	if (_transform_index == IDENTITY) {
		return d.translate (- _position);
	}

	Transform const & transform = _parent->transform (_transform_index);
	return d.translate (- _position - transform.translation).scale (Duple (1, 1) / transform.scale);
}

Rect
Item::parent_to_item (Rect const & r) const
{
	if (_transform_index == IDENTITY) {
		return r.translate (- _position);
	}

	Transform const & transform = _parent->transform (_transform_index);
	return r.translate (- _position - transform.translation).scale (Duple (1, 1) / transform.scale);
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
Item::bbox () const
{
	if (_bbox_dirty) {
		compute_bbox ();
	}

	assert (!_bbox_dirty);
	return _bbox;
}

void
Item::begin_change ()
{
	if (bbox ()) {
		_pre_parent_bbox = item_to_parent (bbox().get ());
	} else {
		_pre_parent_bbox.reset ();
	}
}

void
Item::end_change ()
{
	_canvas->item_changed (this, _pre_parent_bbox);
	
	if (_parent) {
		_parent->child_changed ();
	}
}

void
Item::move (Duple movement)
{
	set_position (position() + movement);
}

void
Item::add_item_state (XMLNode* node) const
{
	node->add_property ("x-position", string_compose ("%1", _position.x));
	node->add_property ("y-position", string_compose ("%1", _position.y));
	node->add_property ("visible", _visible ? "yes" : "no");
}

void
Item::set_item_state (XMLNode const * node)
{
	_position.x = atof (node->property("x-position")->value().c_str());
	_position.y = atof (node->property("y-position")->value().c_str());
	_visible = string_is_affirmative (node->property("visible")->value());
}

void
Item::grab ()
{
	assert (_canvas);
	_canvas->grab (this);
}

void
Item::ungrab ()
{
	assert (_canvas);
	_canvas->ungrab ();
}

void
Item::set_data (string const & key, void* data)
{
	_data[key] = data;
}

void *
Item::get_data (string const & key) const
{
	map<string, void*>::const_iterator i = _data.find (key);
	if (i == _data.end ()) {
		return 0;
	}
	
	return i->second;
}

void
Item::item_to_canvas (Coord& x, Coord& y) const
{
	Duple d (x, y);
	Item const * i = this;
	
	while (i) {
		d = i->item_to_parent (d);
		i = i->_parent;
	}
		
	x = d.x;
	y = d.y;
}

void
Item::canvas_to_item (Coord& x, Coord& y) const
{
	Duple d (x, y);
	Item const * i = this;

	while (i) {
		d = i->parent_to_item (d);
		i = i->_parent;
	}

	x = d.x;
	y = d.y;
}

Rect
Item::item_to_canvas (Rect const & area) const
{
	Rect r = area;
	Item const * i = this;

	while (i) {
		r = i->item_to_parent (r);
		i = i->parent ();
	}

	return r;
}

void
Item::set_ignore_events (bool ignore)
{
	_ignore_events = ignore;
}
