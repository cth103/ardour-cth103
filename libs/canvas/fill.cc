#include "canvas/fill.h"

using namespace ArdourCanvas;

Fill::Fill (Group* parent)
	: Item (parent)
	, _fill_color (0x000000ff)
	, _fill (true)
{

}

void
Fill::set_fill_color (uint32_t color)
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
