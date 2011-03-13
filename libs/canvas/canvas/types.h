#ifndef __CANVAS_TYPES_H__
#define __CANVAS_TYPES_H__

#include <iostream>

namespace ArdourCanvas
{

typedef double Coord;
typedef double Distance;

struct Duple
{
	Duple ()
		: x (0)
		, y (0)
	{}
	
	Duple (Coord x_, Coord y_)
		: x (x_)
		, y (y_)
	{}
		     
	Coord x;
	Coord y;
};

extern Duple operator- (Duple const & o);

struct Rect
{
	Rect ()
		: x0 (0)
		, y0 (0)
		, x1 (0)
		, y1 (0)
	{}
	
	Rect (Coord x0_, Coord y0_, Coord x1_, Coord y1_)
		: x0 (x0_)
		, y0 (y0_)
		, x1 (x1_)
		, y1 (y1_)
	{}
		
	Coord x0;
	Coord y0;
	Coord x1;
	Coord y1;

	Rect intersection (Rect const &) const;
	void extend (Rect const &);
	void translate (Duple const &);

	Distance width () const {
		return x1 - x0;
	}

	Distance height () const {
		return y1 - y0;
	}
};

extern std::ostream & operator<< (std::ostream &, Rect const &);

}
	
#endif
