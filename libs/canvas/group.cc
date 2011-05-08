#include <iostream>
#include <cairomm/context.h>
#include "pbd/stacktrace.h"
#include "pbd/xml++.h"
#include "canvas/group.h"
#include "canvas/types.h"
#include "canvas/debug.h"
#include "canvas/item_factory.h"

using namespace std;
using namespace ArdourCanvas;

int Group::default_items_per_cell = 64;

Group::Group (Canvas* canvas)
	: Item (canvas)
{
	
}

Group::Group (Group* parent)
	: Item (parent)
{

}

Group::Group (Group* parent, Duple position)
	: Item (parent, position)
{

}

Group::~Group ()
{
	for (vector<Item*>::iterator i = _items.begin(); i != _items.end(); ++i) {
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
	for (vector<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {

		if (!(*i)->visible ()) {
			continue;
		}
		
		boost::optional<Rect> item_bbox = (*i)->bbox ();
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

			if ((*i)->transform_index() != IDENTITY) {
				Transform const & t = transform ((*i)->transform_index ());
				context->scale (t.scale.x, t.scale.y);
				context->translate (t.translation.x, t.translation.y);
			}

			context->translate ((*i)->position().x, (*i)->position().y);
			
			(*i)->render (r.get(), context);
			context->restore ();
			++render_count;
		}
	}
}

void
Group::compute_bbox () const
{
	Rect bbox;
	bool have_one = false;

	for (vector<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {

		boost::optional<Rect> item_bbox = (*i)->bbox ();
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
		_bbox = boost::optional<Rect> ();
	} else {
		_bbox = bbox;
	}

	_bbox_dirty = false;
}

void
Group::add (Item* i)
{
	_items.push_back (i);
	_bbox_dirty = true;
	
	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: group add\n");
}

void
Group::remove (Item* i)
{
	vector<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());
	_items.erase (j);
	
	_bbox_dirty = true;
	
	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: group remove\n");
}

void
Group::raise_child_to_top (Item* i)
{
	vector<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());
	_items.erase (j);

	_items.push_back (i);
}

void
Group::raise_child (Item* i, int levels)
{
	vector<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());

	++j;

	while (levels > 0 && j != _items.end ()) {
		++j;
		--levels;
	}

	_items.insert (j, i);

	j = find (_items.begin(), _items.end(), i);
	_items.erase (j);
}

void
Group::lower_child_to_bottom (Item* i)
{
	vector<Item*>::iterator j = find (_items.begin(), _items.end(), i);
	assert (j != _items.end ());
	_items.erase (j);

	_items.insert (_items.begin (), i);
}

void
Group::child_changed ()
{
	_bbox_dirty = true;

	if (_parent) {
		_parent->child_changed ();
	}
}

void
Group::add_items_at_point (Duple const point, vector<Item const *>& items) const
{
	boost::optional<Rect> const group_bbox = bbox ();
	if (!group_bbox || !group_bbox.get().contains (point)) {
		return;
	}

	Item::add_items_at_point (point, items);

	vector<Item*> our_items;
	
	for (vector<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		boost::optional<Rect> item_bbox = (*i)->bbox ();
		if (item_bbox) {
			Rect parent_bbox = (*i)->item_to_parent (item_bbox.get ());
			if (parent_bbox.contains (point)) {
				our_items.push_back (*i);
			}
		}
	}
	
	for (vector<Item*>::iterator i = our_items.begin(); i != our_items.end(); ++i) {
		(*i)->add_items_at_point (point - (*i)->position(), items);
	}
}

XMLNode *
Group::get_state () const
{
	XMLNode* node = new XMLNode ("Group");
	for (vector<Item*>::const_iterator i = _items.begin(); i != _items.end(); ++i) {
		node->add_child_nocopy (*(*i)->get_state ());
	}

	add_item_state (node);
	return node;
}

void
Group::set_state (XMLNode const * node)
{
	set_item_state (node);

	XMLNodeList const & children = node->children ();
	for (XMLNodeList::const_iterator i = children.begin(); i != children.end(); ++i) {
		/* this will create the item and add it to this group */
		create_item (this, *i);
	}
}

TransformIndex
Group::add_transform (Transform const & transform)
{
	_transforms.push_back (transform);
	return _transforms.size() - 1;
}

void
Group::set_transform (TransformIndex index, Transform const & transform)
{
	assert (index >= 0 && index < int (_transforms.size()));

	begin_change ();
	
	_transforms[index] = transform;
	_bbox_dirty = true;

	end_change ();
}

Transform const &
Group::transform (TransformIndex index) const
{
	assert (index >= 0 && index < _transforms.size());
	
	return _transforms[index];
}
