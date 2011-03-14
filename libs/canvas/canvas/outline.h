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

protected:
	uint32_t _outline_color;
	Distance _outline_width;
};

}

#endif
