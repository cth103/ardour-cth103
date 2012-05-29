/*
    Copyright (C) 2009 Paul Davis
    Author: Hans Baier

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

#ifndef CANVAS_SYSEX_H_
#define CANVAS_SYSEX_H_

#include <string>

class MidiRegionView;

namespace Canvas {
	class Flag;
}

class SysEx
{
public:
	SysEx (
			MidiRegionView& region,
			Canvas::Group* parent,
			std::string&    text,
			double          height,
			double          x,
			double          y);

	~SysEx ();

	void hide ();
	void show ();

private:	
	bool event_handler (GdkEvent* ev);

	MidiRegionView& _region;
	Canvas::Flag* _flag;
};

#endif /* __SYSEX_H__ */
