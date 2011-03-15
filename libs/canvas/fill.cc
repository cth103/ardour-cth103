#include "canvas/fill.h"

using namespace ArdourCanvas;

Fill::Fill ()
	: _fill_color (0x000000ff)
	, _fill (true)
{

}

void
Fill::set_fill_color (uint32_t color)
{
	_fill_color = color;
}

void
Fill::set_fill (bool fill)
{
	_fill = fill;
}
