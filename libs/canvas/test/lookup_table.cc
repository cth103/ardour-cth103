#include "canvas/lookup_table.h"
#include "canvas/types.h"
#include "canvas/rectangle.h"
#include "canvas/group.h"
#include "canvas/canvas.h"
#include "lookup_table.h"

using namespace std;
using namespace ArdourCanvas;

CPPUNIT_TEST_SUITE_REGISTRATION (LookupTableTest);

void
LookupTableTest::build_1 ()
{
	ImageCanvas canvas;
	RootGroup group (&canvas);
	Rectangle a (&group, Rect (0, 0, 32, 32));
	a.set_outline_width (0);
	Rectangle b (&group, Rect (0, 33, 32, 64));
	b.set_outline_width (0);
	Rectangle c (&group, Rect (33, 0, 64, 32));
	c.set_outline_width (0);
	Rectangle d (&group, Rect (33, 33, 64, 64));
	d.set_outline_width (0);
	LookupTable table (group, 1);
	
	CPPUNIT_ASSERT (table._items_per_cell == 1);
	CPPUNIT_ASSERT (table._cell_size.x == 32);
	CPPUNIT_ASSERT (table._cell_size.y == 32);
	CPPUNIT_ASSERT (table._cells[0][0].front() == &a);
	CPPUNIT_ASSERT (table._cells[0][1].front() == &b);
	CPPUNIT_ASSERT (table._cells[1][0].front() == &c);
	CPPUNIT_ASSERT (table._cells[1][1].front() == &d);
}

void
LookupTableTest::build_2 ()
{
	ImageCanvas canvas;
	RootGroup group (&canvas);
	Rectangle a (&group, Rect (0, 0, 713, 1024));
	a.set_outline_width (0);
	Rectangle b (&group, Rect (0, 0, 0, 1024));
	b.set_outline_width (0);
	LookupTable table (group, 64);
}

void
LookupTableTest::build_negative ()
{
	ImageCanvas canvas;
	RootGroup group (&canvas);
	Rectangle a (&group, Rect (-32, -32, 32, 32));
	LookupTable table (group, 1);
}

void
LookupTableTest::get_small ()
{
	ImageCanvas canvas;
	RootGroup group (&canvas);
	Rectangle a (&group, Rect (0, 0, 32, 32));
	a.set_outline_width (0);
	Rectangle b (&group, Rect (0, 33, 32, 64));
	b.set_outline_width (0);
	Rectangle c (&group, Rect (33, 0, 64, 32));
	c.set_outline_width (0);
	Rectangle d (&group, Rect (33, 33, 64, 64));
	d.set_outline_width (0);
	LookupTable table (group, 1);
	
	list<Item*> items = table.get (Rect (16, 16, 48, 48));
	CPPUNIT_ASSERT (items.size() == 4);
	
	items = table.get (Rect (32, 32, 33, 33));
	CPPUNIT_ASSERT (items.size() == 1);
}

void
LookupTableTest::get_big ()
{
	ImageCanvas canvas;
	RootGroup group (&canvas);

	double const s = 8;
	int const N = 1024;
	
	for (int x = 0; x < N; ++x) {
		for (int y = 0; y < N; ++y) {
			Rectangle* r = new Rectangle (&group);
			r->set_outline_width (0);
			r->set (Rect (x * s, y * s, (x + 1) * s, (y + 1) * s));
		}
	}

	LookupTable table (group, 16);
	list<Item*> items = table.get (Rect (0, 0, 15, 15));
	CPPUNIT_ASSERT (items.size() == 16);
}
