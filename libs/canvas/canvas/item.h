#ifndef __CANVAS_ITEM_H__
#define __CANVAS_ITEM_H__

#include <cairomm/context.h>

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

	/** @param area Area in parent's coordinates */
	virtual void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const = 0;

	/** @return Bounding box in parent's coordinates */
	virtual Rect bounding_box () const = 0;
	
protected:
	Group* _parent;
};

}

#endif
