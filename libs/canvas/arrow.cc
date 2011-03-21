#include "canvas/arrow.h"
#include "canvas/polygon.h"
#include "canvas/line.h"

using namespace ArdourCanvas;

Arrow::Arrow (Group* parent)
	: Group (parent)
{
	for (int i = 0; i < 2; ++i) {
		_heads[i].polygon = new Polygon (this);
		_heads[i].show = true;
		_heads[i].outward = true;
		_heads[i].width = 4;
		_heads[i].height = 4;
		setup_polygon (i);
	}
	
	_line = new Line (this);
}

void
Arrow::set_show_head (int which, bool show)
{
	begin_change ();
	
	assert (which == 0 || which == 1);
	_heads[which].show = show;

	setup_polygon (which);
	_bounding_box_dirty = true;
	end_change ();
}

void
Arrow::set_head_outward (int which, bool outward)
{
	begin_change ();

	assert (which == 0 || which == 1);
	_heads[which].outward = outward;

	setup_polygon (which);
	_bounding_box_dirty = true;
	end_change ();
}

void
Arrow::set_head_height (int which, Distance height)
{
	begin_change ();
	
	assert (which == 0 || which == 1);
	_heads[which].height = height;

	setup_polygon (which);
	_bounding_box_dirty = true;
	end_change ();
}

void
Arrow::set_head_width (int which, Distance width)
{
	begin_change ();
	
	assert (which == 0 || which == 1);
	_heads[which].width = width;

	setup_polygon (which);
	_bounding_box_dirty = true;
	end_change ();
}

void
Arrow::set_outline_width (Distance width)
{
	_line->set_outline_width (width);
}

void
Arrow::set_x (Coord x)
{
	_line->set_x0 (x);
	_line->set_x1 (x);
	for (int i = 0; i < 2; ++i) {
		_heads[i].polygon->set_x_position (x - _heads[i].width / 2);
	}
		
}

void
Arrow::set_y0 (Coord y0)
{
	_line->set_y0 (y0);
	_heads[0].polygon->set_y_position (y0);
}

void
Arrow::set_y1 (Coord y1)
{
	_line->set_y1 (y1);
	_heads[1].polygon->set_y_position (y1 - _heads[1].height);
}

Coord
Arrow::x () const
{
	return _line->x0 ();
}

Coord
Arrow::y1 () const
{
	return _line->y1 ();
}

void
Arrow::setup_polygon (int which)
{
	Points points;

	if ((which == 0 && _heads[which].outward) || (which == 1 && !_heads[which].outward)) {
		points.push_back (Duple (_heads[which].width / 2, 0));
		points.push_back (Duple (_heads[which].width, _heads[which].height));
		points.push_back (Duple (0, _heads[which].height));
	} else {
		points.push_back (Duple (0, 0));
		points.push_back (Duple (_heads[which].width, 0));
		points.push_back (Duple (_heads[which].width / 2, _heads[which].height));
	}

	_heads[which].polygon->set (points);
}

void
Arrow::set_color (uint32_t color)
{
	_line->set_outline_color (color);
	for (int i = 0; i < 2; ++i) {
		_heads[i].polygon->set_outline_color (color);
		_heads[i].polygon->set_fill_color (color);
	}
}
