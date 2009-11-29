/*
    Copyright (C) 2009 Paul Davis

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

#ifndef __ardour_debug_h__
#define __ardour_debug_h__

#include <stdint.h>

#include <sstream>

namespace ARDOUR {

	extern uint64_t debug_bits;
	void debug_print (const char* prefix, std::string str);
	void set_debug_bits (uint64_t bits);

	namespace DEBUG {

		/* this namespace is so that we can write DEBUG::bit_name */

		enum DebugBits {
			MidiSourceIO = 0x1,
			MidiPlaylistIO = 0x2,
			MidiDiskstreamIO = 0x4,
			SnapBBT = 0x8,
			Configuration = 0x10,
			Latency = 0x20,
			Processors = 0x40,
			Graph = 0x80,
			Destruction = 0x100,
			MTC = 0x200
		};
	}

}

#ifndef NDEBUG
#define DEBUG_TRACE(bits,str) if ((bits) & ARDOUR::debug_bits) { ARDOUR::debug_print (# bits, str); }
#define DEBUG_STR_SET(id,s) std::stringstream __debug_str ## id; __debug_str ## id << s;
#define DEBUG_STR(id) __debug_str ## id
#else
#define DEBUG_TRACE(bits,fmt,...) /*empty*/
#define DEBUG_STR_SET(a,b) /* empty */
#define DEBUG_STR(a) /* empty */
#endif

#endif /* __ardour_debug_h__ */

