#ifndef __CANVAS_POLY_ITEM_H__
#define __CANVAS_POLY_ITEM_H__

#include "canvas/item.h"

namespace ArdourCanvas {

class PolyItem : virtual public Item
{
public:
	PolyItem (Group *);

	void compute_bounding_box () const;
	
	void set (Points const &);
	Points const & get () const;

protected:
	void render_path (Rect const &, Cairo::RefPtr<Cairo::Context>) const;

	Points _points;
};
	
}

#endif
