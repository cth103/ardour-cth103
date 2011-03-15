#ifndef __CANVAS_POLYLINE_H__
#define __CANVAS_POLYLINE_H__

#include "canvas/item.h"

namespace ArdourCanvas {

class PolyLine : public Item, public Outline
{
public:
	PolyLine (Group *);

	boost::optional<Rect> bounding_box () const;
	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	
	void set (Points const &);
	Points const & get () const;

#ifdef CANVAS_COMPATIBILITY
	Points& property_points () {
		return _points;
	}
#endif	

private:
	Points _points;
};
	
}

#endif
