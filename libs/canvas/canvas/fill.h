#ifndef __CANVAS_FILL_H__
#define __CANVAS_FILL_H__

#include <stdint.h>

namespace ArdourCanvas {

class Fill
{
public:
	uint32_t fill_color () const;
	void set_fill_color (uint32_t);

#ifdef CANVAS_COMPATIBILITY
	uint32_t& property_fill_color () {
		return _fill_color;
	}

	uint32_t& property_fill_color_rgba () {
		return _fill_color;
	}
#endif
	
protected:
	uint32_t _fill_color;
};

}

#endif
