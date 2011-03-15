#include "canvas/lookup_table.h"
#include "canvas/types.h"
#include "canvas/rectangle.h"
#include "canvas/group.h"
#include "lookup_table.h"

using namespace std;
using namespace ArdourCanvas;

CPPUNIT_TEST_SUITE_REGISTRATION (LookupTableTest);

void
LookupTableTest::build ()
{
	Group group;
	Rectangle a (&group, Rect (0, 0, 32, 32));
	Rectangle b (&group, Rect (0, 33, 32, 64));
	Rectangle c (&group, Rect (33, 0, 64, 32));
	Rectangle d (&group, Rect (33, 33, 64, 64));
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
LookupTableTest::build_negative ()
{
	Group group;
	Rectangle a (&group, Rect (-32, -32, 32, 32));
	LookupTable table (group, 1);
}

void
LookupTableTest::get_small ()
{
	Group group;
	Rectangle a (&group, Rect (0, 0, 32, 32));
	Rectangle b (&group, Rect (0, 33, 32, 64));
	Rectangle c (&group, Rect (33, 0, 64, 32));
	Rectangle d (&group, Rect (33, 33, 64, 64));
	LookupTable table (group, 1);
	
	list<Item*> items = table.get (Rect (16, 16, 48, 48));
	CPPUNIT_ASSERT (items.size() == 4);
	
	items = table.get (Rect (32, 32, 33, 33));
	CPPUNIT_ASSERT (items.size() == 1);
}

void
LookupTableTest::get_big ()
{
	Group group;

	double const s = 8;
	int const N = 1024;
	
	for (int x = 0; x < N; ++x) {
		for (int y = 0; y < N; ++y) {
			Rectangle* r = new Rectangle (&group);
			r->set (Rect (x * s, y * s, (x + 1) * s, (y + 1) * s));
		}
	}

	LookupTable table (group, 16);
	list<Item*> items = table.get (Rect (0, 0, 15, 15));
	CPPUNIT_ASSERT (items.size() == 16);
}
