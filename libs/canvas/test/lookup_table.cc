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
	Rectangle a (canvas.root(), Rect (0, 0, 32, 32));
	a.set_outline_width (0);
	Rectangle b (canvas.root(), Rect (0, 33, 32, 64));
	b.set_outline_width (0);
	Rectangle c (canvas.root(), Rect (33, 0, 64, 32));
	c.set_outline_width (0);
	Rectangle d (canvas.root(), Rect (33, 33, 64, 64));
	d.set_outline_width (0);
	LookupTable table (*canvas.root(), 1);
	
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
	Rectangle a (canvas.root(), Rect (0, 0, 713, 1024));
	a.set_outline_width (0);
	Rectangle b (canvas.root(), Rect (0, 0, 0, 1024));
	b.set_outline_width (0);
	LookupTable table (*canvas.root(), 64);
}

void
LookupTableTest::build_negative ()
{
	ImageCanvas canvas;
	Rectangle a (canvas.root(), Rect (-32, -32, 32, 32));
	LookupTable table (*canvas.root(), 1);
}

void
LookupTableTest::get_small ()
{
	ImageCanvas canvas;
	Rectangle a (canvas.root(), Rect (0, 0, 32, 32));
	a.set_outline_width (0);
	Rectangle b (canvas.root(), Rect (0, 33, 32, 64));
	b.set_outline_width (0);
	Rectangle c (canvas.root(), Rect (33, 0, 64, 32));
	c.set_outline_width (0);
	Rectangle d (canvas.root(), Rect (33, 33, 64, 64));
	d.set_outline_width (0);
	LookupTable table (*canvas.root(), 1);
	
	vector<Item*> items = table.get (Rect (16, 16, 48, 48));
	CPPUNIT_ASSERT (items.size() == 4);
	
	items = table.get (Rect (32, 32, 33, 33));
	CPPUNIT_ASSERT (items.size() == 1);
}

void
LookupTableTest::get_big ()
{
	ImageCanvas canvas;

	double const s = 8;
	int const N = 1024;
	
	for (int x = 0; x < N; ++x) {
		for (int y = 0; y < N; ++y) {
			Rectangle* r = new Rectangle (canvas.root());
			r->set_outline_width (0);
			r->set (Rect (x * s, y * s, (x + 1) * s, (y + 1) * s));
		}
	}

	LookupTable table (*canvas.root(), 16);
	vector<Item*> items = table.get (Rect (0, 0, 15, 15));
	CPPUNIT_ASSERT (items.size() == 16);
}
