#include "canvas/group.h"
#include "canvas/types.h"
#include "canvas/rectangle.h"
#include "group.h"

using namespace ArdourCanvas;

CPPUNIT_TEST_SUITE_REGISTRATION (GroupTest);

void
GroupTest::bounding_box ()
{
	Group group;
	Rectangle a (&group, Rect (0, 0, 32, 32));
	Rectangle b (&group, Rect (0, 33, 32, 64));
	Rectangle c (&group, Rect (33, 0, 64, 32));
	Rectangle d (&group, Rect (33, 33, 64, 64));
	boost::optional<Rect> bbox = group.bounding_box ();
	
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 64);
	CPPUNIT_ASSERT (bbox.get().y1 == 64);
}
