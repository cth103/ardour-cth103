#include "canvas/group.h"

#ifndef __CANVAS_ARROW_H__
#define __CANVAS_ARROW_H__

namespace ArdourCanvas {

class Line;
class Polygon;

/* XXX: vertical only */
class Arrow : public Group
{
public:
	Arrow (Group *);

	void set_show_head (int, bool);
	void set_head_outward (int, bool);
	void set_head_height (int, Distance);
	void set_head_width (int, Distance);
	void set_outline_width (Distance);
	void set_color (uint32_t);

	Coord x () const;
	Coord y1 () const;

	void set_x (Coord);
	void set_y0 (Coord);
	void set_y1 (Coord);
	
private:
	void setup_polygon (int);
	
	struct Head {
		Polygon* polygon;
		bool show;
		bool outward;
		Distance height;
		Distance width;
	};

	Head _heads[2];

	Line* _line;
};

}

#endif
