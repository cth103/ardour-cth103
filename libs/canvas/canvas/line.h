#ifndef __CANVAS_LINE_H__
#define __CANVAS_LINE_H__

#include "canvas/item.h"
#include "canvas/outline.h"
#include "canvas/poly_line.h"

namespace ArdourCanvas {

class Line : public Item, public Outline
{
public:
	Line (Group *);

	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	
	void set (Point, Point);
	void set_x0 (Coord);
	void set_y0 (Coord);
	void set_x1 (Coord);
	void set_y1 (Coord);
	Coord x0 () const {
		return _points[0].x;
	}
	Coord y0 () const {
		return _points[0].y;
	}
	Coord x1 () const {
		return _points[1].x;
	}
	Coord y1 () const {
		return _points[1].y;
	}

#ifdef CANVAS_COMPATIBILITY
	Coord& property_x1 () {
		return _points[0].x;
	}

	Coord& property_x2 () {
		return _points[1].x;
	}

	Coord& property_y1 () {
		return _points[0].y;
	}

	Coord& property_y2 () {
		return _points[1].y;
	}
#endif

private:
	Point _points[2];
};
	
}

#endif
