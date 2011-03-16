#ifndef __CANVAS_OUTLINE_H__
#define __CANVAS_OUTLINE_H__

#include <stdint.h>
#include "canvas/types.h"
#include "canvas/item.h"

namespace ArdourCanvas {

class Outline : virtual public Item
{
public:
	Outline (Group *);
	virtual ~Outline () {}
	
	uint32_t outline_color () const {
		return _outline_color;
	}

	virtual void set_outline_color (uint32_t);

	Distance outline_width () const {
		return _outline_width;
	}
	
	virtual void set_outline_width (Distance);

#ifdef CANVAS_COMPATIBILITY
	uint32_t& property_outline_color_rgba () {
		return _outline_color;
	}

	uint32_t& property_color_rgba () {
		return _outline_color;
	}
	
	double& property_outline_pixels () {
		return _outline_width;
	}

	double& property_width_pixels () {
		return _outline_width;
	}

	int& property_first_arrowhead () {
		return _foo_int;
	}
	int& property_last_arrowhead () {
		return _foo_int;
	}
	int& property_arrow_shape_a () {
		return _foo_int;
	}
	int& property_arrow_shape_b () {
		return _foo_int;
	}
	int& property_arrow_shape_c () {
		return _foo_int;
	}
	bool& property_draw () {
		return _foo_bool;
	}
#endif	

protected:

	void setup_outline_context (Cairo::RefPtr<Cairo::Context>) const;
	
	uint32_t _outline_color;
	Distance _outline_width;

#ifdef CANVAS_COMPATIBILITY
	int _foo_int;
	bool _foo_bool;
#endif
};

}

#endif
