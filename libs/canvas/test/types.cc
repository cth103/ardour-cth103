#include "canvas/types.h"
#include "types.h"

using namespace std;
using namespace ArdourCanvas;

CPPUNIT_TEST_SUITE_REGISTRATION (TypesTest);

void
TypesTest::intersect ()
{
	{
		Rect a (0, 0, 1024, 1024);
		Rect b (0, 0, 512, 512);
		boost::optional<Rect> c = a.intersection (b);

		CPPUNIT_ASSERT (c.is_initialized ());
		CPPUNIT_ASSERT (c->x0 == 0);
		CPPUNIT_ASSERT (c->x1 == 512);
		CPPUNIT_ASSERT (c->y0 == 0);
		CPPUNIT_ASSERT (c->y1 == 512);
	}

	{
		Rect a (0, 0, 512, 512);
		Rect b (513, 513, 1024, 1024);
		boost::optional<Rect> c = a.intersection (b);

		CPPUNIT_ASSERT (!c.is_initialized ());
	}
}
