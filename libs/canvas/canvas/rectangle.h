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

	void compute_bounding_box () const;

	Rect const & get () const {
		return _rect;
	}

	Coord x0 () const {
		return _rect.x0;
	}

	Coord y0 () const {
		return _rect.y0;
	}

	Coord x1 () const {
		return _rect.x1;
	}

	Coord y1 () const {
		return _rect.y1;
	}

	void set (Rect const &);
	void set_x0 (Coord);
	void set_y0 (Coord);
	void set_x1 (Coord);
	void set_y1 (Coord);

private:
	Rect _rect;
};

}

#endif
