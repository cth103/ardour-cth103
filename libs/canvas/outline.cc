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

/** @file  canvas/outline.cc
 *  @brief Implementation of a mixin for items which have an outline.
 */

#include <cairomm/context.h>
#include "pbd/xml++.h"
#include "pbd/compose.h"
#include "ardour/utils.h"
#include "canvas/outline.h"
#include "canvas/utils.h"
#include "canvas/debug.h"

using namespace ArdourCanvas;

/** Construct an Outline object.
 *  @param parent Parent group
 */
Outline::Outline (Group* parent)
	: Item (parent)
	, _outline_color (0x000000ff)
	, _outline_width (0.5)
	, _outline (true)
{

}

/** Construct an Outline object.
 *  @param parent Parent group
 *  @param transform_index Index of the parent transform to apply to this item.
 */
Outline::Outline (Group* parent, TransformIndex transform_index)
	: Item (parent, transform_index)
	, _outline_color (0x000000ff)
	, _outline_width (0.5)
	, _outline (true)
{

}

/** Set the color to use for this item's outline.
 *  @param color New color.
 */
void
Outline::set_outline_color (Color color)
{
	begin_change ();
	
	_outline_color = color;

	end_change ();
}

/** Set the width of this item's outline in pixels.
 *  @param width New width in pixels.
 */
void
Outline::set_outline_width (Distance width)
{
	begin_change ();
	
	_outline_width = width;

	_bbox_dirty = true;
	end_change ();

	DEBUG_TRACE (PBD::DEBUG::CanvasItemsDirtied, "canvas item dirty: outline width change\n");	
}

/** Set whether or not to outline this item.
 *  @param outline true to outline, otherwise false.
 */
void
Outline::set_outline (bool outline)
{
	begin_change ();

	_outline = outline;

	_bbox_dirty = true;
	end_change ();
}

/** Set up a Cairo context to outline this item according
 *  to its settings.
 *  @param context Context to set up.
 */
void
Outline::setup_outline_context (Cairo::RefPtr<Cairo::Context> context) const
{
	set_source_rgba (context, _outline_color);
	context->set_line_width (_outline_width);
}

/** Add our state to an XML node.
 *  @param node XML node to add to.
 */
void
Outline::add_outline_state (XMLNode* node) const
{
	node->add_property ("outline-color", string_compose ("%1", _outline_color));
	node->add_property ("outline", _outline ? "yes" : "no");
	node->add_property ("outline-width", string_compose ("%1", _outline_width));
}

/** Set our state from an XML node.
 *  @param node XML node to get state from.
 */
void
Outline::set_outline_state (XMLNode const * node)
{
	_outline_color = atoll (node->property("outline-color")->value().c_str());
	_outline = string_is_affirmative (node->property("outline")->value().c_str());
	_outline_width = atof (node->property("outline-width")->value().c_str());

	_bbox_dirty = true;
}
