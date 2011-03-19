#ifndef __CANVAS_DEBUG_H__
#define __CANVAS_DEBUG_H__

#include "pbd/debug.h"

namespace PBD {
	namespace DEBUG {
		extern uint64_t CanvasItems;
		extern uint64_t CanvasItemsDirtied;
		extern uint64_t CanvasEvents;
	}
}

#ifdef CANVAS_DEBUG
#define CANVAS_DEBUG_NAME(i, n) i->name = n;
#else
#define CANVAS_DEBUG(i, n) /* empty */
#endif

#endif
