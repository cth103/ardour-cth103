#include <algorithm>
#include <cassert>
#include "canvas/types.h"

using namespace std;
using namespace ArdourCanvas;

boost::optional<Rect>
Rect::intersection (Rect const & o) const
{
	Rect i;
	
	i.x0 = max (x0, o.x0);
	i.y0 = max (y0, o.y0);
	i.x1 = min (x1, o.x1);
	i.y1 = min (y1, o.y1);

	if (i.x0 > i.x1 || i.y0 > i.y1) {
		return boost::optional<Rect> ();
	}
	
	return boost::optional<Rect> (i);
}

void
Rect::translate (Duple const & t)
{
	x0 += t.x;
	y0 += t.y;
	x1 += t.x;
	y1 += t.y;
}

void
Rect::extend (Rect const & o)
{
	x0 = min (x0, o.x0);
	y0 = min (y0, o.y0);
	x1 = max (x1, o.x1);
	y1 = max (y1, o.y1);
}

Duple
ArdourCanvas::operator- (Duple const & o)
{
	return Duple (-o.x, -o.y);
}

ostream &
ArdourCanvas::operator<< (ostream & s, Duple const & r)
{
	s << "(" << r.x << ", " << r.y << ")";
	return s;
}

ostream &
ArdourCanvas::operator<< (ostream & s, Rect const & r)
{
	s << "[(" << r.x0 << ", " << r.y0 << "), (" << r.x1 << ", " << r.y1 << ")]";
	return s;
}

