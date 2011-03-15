#ifndef __CANVAS_RECTANGLE_H__
#define __CANVAS_RECTANGLE_H__

#include "canvas/item.h"
#include "canvas/types.h"
#include "canvas/outline.h"
#include "canvas/fill.h"

namespace ArdourCanvas
{

class Rectangle : public Item, public Outline, public Fill
{
public:
	Rectangle (Group *);
	Rectangle (Group *, Rect const &);

	void render (Rect const &, Cairo::RefPtr<Cairo::Context>) const;
	boost::optional<Rect> bounding_box () const;

	void set (Rect const &);

#ifdef CANVAS_COMPATIBILITY
	Coord& property_x1 () {
		return _rect.x0;
	}

	Coord& property_x2 () {
		return _rect.x1;
	}

	Coord& property_y1 () {
		return _rect.y0;
	}

	Coord& property_y2 () {
		return _rect.y1;
	}
#endif	
	
private:
	Rect _rect;
};

}

#endif
