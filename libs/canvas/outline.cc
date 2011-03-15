#include <cairomm/context.h>
#include "canvas/outline.h"
#include "canvas/utils.h"

using namespace ArdourCanvas;

Outline::Outline ()
	: _outline_color (0x000000ff)
	, _outline_width (1)
{

}

void
Outline::set_outline_color (uint32_t color)
{
	_outline_color = color;
}

void
Outline::set_outline_width (Distance width)
{
	_outline_width = width;
}

void
Outline::setup_context (Cairo::RefPtr<Cairo::Context> context) const
{
	set_source_rgba (context, _outline_color);
	context->set_line_width (_outline_width);
}
