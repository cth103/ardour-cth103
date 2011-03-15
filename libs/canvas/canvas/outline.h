#ifndef __CANVAS_OUTLINE_H__
#define __CANVAS_OUTLINE_H__

#include <stdint.h>
#include "canvas/types.h"

namespace ArdourCanvas {

class Outline
{
public:
	Outline ();
	
	uint32_t outline_color () const {
		return _outline_color;
	}

	void set_outline_color (uint32_t);

	Distance outline_width () const {
		return _outline_width;
	}
	
	void set_outline_width (Distance);

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

	int& property_outline_what ();

	int& property_first_arrowhead ();
	int& property_last_arrowhead ();
	int& property_arrow_shape_a ();
	int& property_arrow_shape_b ();
	int& property_arrow_shape_c ();

	bool& property_draw ();
#endif	

protected:
	uint32_t _outline_color;
	Distance _outline_width;
};

}

#endif
