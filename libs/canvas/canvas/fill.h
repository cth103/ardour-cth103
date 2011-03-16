#ifndef __CANVAS_FILL_H__
#define __CANVAS_FILL_H__

#include <stdint.h>
#include "canvas/item.h"

namespace ArdourCanvas {

class Fill : virtual public Item
{
public:
	Fill (Group *);
	
	uint32_t fill_color () const;
	void set_fill_color (uint32_t);
	bool fill () const {
		return _fill;
	}
	void set_fill (bool);

#ifdef CANVAS_COMPATIBILITY
	uint32_t& property_fill_color () {
		return _fill_color;
	}

	uint32_t& property_fill_color_rgba () {
		return _fill_color;
	}

	bool& property_fill () {
		return _fill;
	}
#endif
	
protected:
	void setup_fill_context (Cairo::RefPtr<Cairo::Context>) const;
	
	uint32_t _fill_color;
	bool _fill;
};

}

#endif
