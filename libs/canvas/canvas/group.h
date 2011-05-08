#ifndef __CANVAS_GROUP_H__
#define __CANVAS_GROUP_H__

#include <list>
#include <vector>
#include "canvas/item.h"
#include "canvas/types.h"

namespace ArdourCanvas {

class Group : public Item
{
public:
	explicit Group (Group *);
	explicit Group (Group *, Duple);
	~Group ();

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	virtual void compute_bbox () const;
	XMLNode* get_state () const;
	void set_state (XMLNode const *);

	void add (Item *);
	void remove (Item *);
	std::vector<Item*> const & items () const {
		return _items;
	}

	void raise_child_to_top (Item *);
	void raise_child (Item *, int);
	void lower_child_to_bottom (Item *);
	void child_changed ();

	TransformIndex add_transform (Transform const &);
	void set_transform (TransformIndex, Transform const &);
	Transform const & transform (TransformIndex) const;

	void add_items_at_point (Duple, std::vector<Item const *> &) const;

	static int default_items_per_cell;

protected:
	
	explicit Group (Canvas *);
	
private:
	Group (Group const &);

	/* our items, from lowest to highest in the stack */
	std::vector<Item*> _items;

	std::vector<Transform> _transforms;
};

}

#endif
