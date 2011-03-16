#include <cairomm/context.h>
#include "canvas/outline.h"
#include "canvas/utils.h"

using namespace ArdourCanvas;

Outline::Outline (Group* parent)
	: Item (parent)
	, _outline_color (0x000000ff)
	, _outline_width (1)
{

}

void
Outline::set_outline_color (uint32_t color)
{
	begin_change ();
	
	_outline_color = color;

	end_change ();
}

void
Outline::set_outline_width (Distance width)
{
	begin_change ();
	
	_outline_width = width;

	_bounding_box_dirty = true;
	end_change ();
}

void
Outline::setup_outline_context (Cairo::RefPtr<Cairo::Context> context) const
{
	set_source_rgba (context, _outline_color);
	context->set_line_width (_outline_width);
}
