/*
    Copyright (C) 2011 Paul Davis
    Author: Carl Hetherington <cth@carlh.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/

/** @file  canvas/debug.h
 *  @brief Canvas debugging code.
 */

#ifndef __CANVAS_DEBUG_H__
#define __CANVAS_DEBUG_H__

#include <sys/time.h>
#include <map>
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

namespace Canvas {
	extern struct timeval epoch;
	extern std::map<std::string, struct timeval> last_time;
	extern void checkpoint (std::string, std::string);
	extern void set_epoch ();
}

#endif
