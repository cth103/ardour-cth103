namespace ArdourCanvas {

#include "canvas/item.h"
#include "canvas/poly_line.h"

class Line : public Item
{
public:
	Line (Group *);

	void set (Point, Point);

private:
	Point _points[2];
	PolyLine _poly_line;
};
	
}

	
