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
	Group group ((Group *) 0);
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



