#ifndef __CANVAS_POLY_LINE_H__
#define __CANVAS_POLY_LINE_H__

#include "canvas/poly_item.h"
#include "canvas/outline.h"

namespace ArdourCanvas {

class PolyLine : public PolyItem, public Outline
{
public:
	PolyLine (Group *);

	void render (Rect const & area, Cairo::RefPtr<Cairo::Context>) const;
	XMLNode* get_state () const;
};
	
}

#endif
