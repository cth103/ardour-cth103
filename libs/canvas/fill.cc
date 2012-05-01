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

#include "ardour/utils.h"
#include "pbd/xml++.h"
#include "pbd/compose.h"
#include "pbd/convert.h"
#include "canvas/fill.h"
#include "canvas/utils.h"

using namespace std;
using namespace Canvas;

Fill::Fill (Group* parent)
	: Item (parent)
	, _fill_color (0x000000ff)
	, _fill (true)
{

}

Fill::Fill (Group* parent, TransformIndex transform_index)
	: Item (parent, transform_index)
	, _fill_color (0x000000ff)
	, _fill (true)
{

}

void
Fill::set_fill_color (Color color)
{
	begin_change ();
	
	_fill_color = color;

	end_change ();
}

void
Fill::set_fill (bool fill)
{
	begin_change ();
	
	_fill = fill;

	end_change ();
}

void
Fill::setup_fill_context (Cairo::RefPtr<Cairo::Context> context) const
{
	set_source_rgba (context, _fill_color);
}

void
Fill::add_fill_state (XMLNode* node) const
{
	node->add_property ("fill-color", string_compose ("%1", _fill_color));
	node->add_property ("fill", _fill ? "yes" : "no");
}

void
Fill::set_fill_state (XMLNode const * node)
{
	_fill_color = atoll (node->property("fill-color")->value().c_str());
	_fill = PBD::string_is_affirmative (node->property("fill")->value ().c_str());

	_bbox_dirty = true;
}
