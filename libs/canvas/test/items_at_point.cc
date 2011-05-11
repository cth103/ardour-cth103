#include "canvas/group.h"
#include "canvas/canvas.h"
#include "canvas/rectangle.h"
#include "items_at_point.h"

using namespace std;
using namespace Canvas;

CPPUNIT_TEST_SUITE_REGISTRATION (ItemsAtPointTest);

void
ItemsAtPointTest::with_transform ()
{
	ImageCanvas canvas;
	Group group (canvas.root ());
	TransformIndex tx = group.add_transform (Transform (Duple (2, 1), Duple (0, 0)));

	Rectangle rectangle (&group, tx, Rect (0, 0, 16, 16));

	vector<Item const *> items;
	
	canvas.root()->add_items_at_point (Duple (20, 0), items);

	CPPUNIT_ASSERT (items.size() == 3);
	vector<Item const *>::iterator i = items.begin ();
	CPPUNIT_ASSERT (*i++ == canvas.root ());
	CPPUNIT_ASSERT (*i++ == &group);
	CPPUNIT_ASSERT (*i++ == &rectangle);
}
