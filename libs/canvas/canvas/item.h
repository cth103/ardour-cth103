#ifndef __CANVAS_ITEM_H__
#define __CANVAS_ITEM_H__

#include <cairomm/context.h>
#include "canvas/types.h"

namespace ArdourCanvas
{

class Canvas;	
class Group;
class Rect;	

class Item
{
public:
	Item (Group *);
	virtual ~Item ();

	/** Set the position of this item in the parent's coordinates */
	void set_position (Duple);

	Duple position () const {
		return _position;
	}

	/** @return Bounding box in this item's coordinates */
	virtual boost::optional<Rect> bounding_box () const = 0;

	/** @param area Area to draw in this item's coordinates */
	virtual void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const = 0;

	Rect item_to_parent (Rect const &) const;
	
protected:
	Group* _parent;
	/** position of this item in parent coordinates */
	Duple _position;
};

}

#endif
