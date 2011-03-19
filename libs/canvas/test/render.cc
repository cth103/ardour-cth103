#include "canvas/canvas.h"
#include "canvas/line.h"
#include "canvas/rectangle.h"
#include "canvas/polygon.h"
#include "render.h"

using namespace std;
using namespace ArdourCanvas;

CPPUNIT_TEST_SUITE_REGISTRATION (RenderTest);

void
RenderTest::check (string const & name)
{
	stringstream s;
	s << "diff -q " << name << ".png " << "../../libs/canvas/test/" << name << ".png";
	int r = system (s.str().c_str());
	CPPUNIT_ASSERT (WEXITSTATUS (r) == 0);
}

void
RenderTest::basics ()
{
	ImageCanvas canvas (Duple (128, 128));

	/* line */
	Group line_group (canvas.root ());
	line_group.set_position (Duple (0, 0));
	Line line (&line_group);
	line.set (Duple (0, 0), Duple (32, 32));
	line.set_outline_width (2);

	/* rectangle */
	Group rectangle_group (canvas.root ());
	rectangle_group.set_position (Duple (64, 0));
	Rectangle rectangle (&rectangle_group);
	rectangle.set (Rect (0, 0, 32, 32));
	rectangle.set_outline_width (2);
	rectangle.set_outline_color (0x00ff00ff);
	rectangle.set_fill_color (0x0000ffff);

	/* poly line */
	Group poly_line_group (canvas.root ());
	poly_line_group.set_position (Duple (0, 64));
	PolyLine poly_line (&poly_line_group);
	Points points;
	points.push_back (Point (0, 0));
	points.push_back (Point (16, 48));
	points.push_back (Point (32, 32));
	poly_line.set (points);
	poly_line.set_outline_color (0xff0000ff);
	poly_line.set_outline_width (2);

	/* polygon */
	Group polygon_group (canvas.root ());
	polygon_group.set_position (Duple (64, 64));
	Polygon polygon (&polygon_group);
	polygon.set (points);
	polygon.set_outline_color (0xff00ffff);
	polygon.set_fill_color (0xcc00ffff);
	polygon.set_outline_width (2);
	
	canvas.render_to_image (Rect (0, 0, 128, 128));
	canvas.write_to_png ("render_basics.png");

	check ("render_basics");
}
