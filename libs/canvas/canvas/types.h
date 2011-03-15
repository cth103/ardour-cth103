#ifndef __CANVAS_TYPES_H__
#define __CANVAS_TYPES_H__

#include <iostream>
#include <cfloat>
#include <vector>
#include <boost/optional.hpp>

namespace ArdourCanvas
{

typedef double Coord;
#define COORD_EPSILON DBL_EPSILON
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

	Duple translate (Duple) const;
};


extern Duple operator- (Duple const & o);
extern std::ostream & operator<< (std::ostream &, Duple const &);

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

	boost::optional<Rect> intersection (Rect const &) const;
	Rect extend (Rect const &) const;
	Rect translate (Duple const &) const;

	Distance width () const {
		return x1 - x0;
	}

	Distance height () const {
		return y1 - y0;
	}
};

extern std::ostream & operator<< (std::ostream &, Rect const &);


#ifdef CANVAS_COMPATIBILITY

class Point
{
public:	
	Point ()
		: x (0)
		, y (0)
	{}
	
	Point (Coord x_, Coord y_)
		: x (x_)
		, y (y_)
	{}

	Coord get_x () const {
		return x;
	}

	Coord get_y () const {
		return y;
	}

	void set_x (Coord x_) {
		x = x_;
	}

	void set_y (Coord y_) {
		y = y_;
	}
	
private:	
	Coord x;
	Coord y;
};

typedef std::vector<Point> Points;

#endif

}
	
#endif
