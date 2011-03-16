#include "canvas/group.h"
#include "canvas/types.h"
#include "canvas/rectangle.h"
#include "canvas/canvas.h"
#include "group.h"

using namespace std;
using namespace ArdourCanvas;

CPPUNIT_TEST_SUITE_REGISTRATION (GroupTest);

extern Rectangle* shitcunt;

void
GroupTest::bounding_box ()
{
	ImageCanvas canvas;
	Group group (&canvas);
	Rectangle a (&group, Rect (0, 0, 32, 32));
	a.set_outline_width (0);
	Rectangle b (&group, Rect (0, 33, 32, 64));
	b.set_outline_width (0);
	Rectangle c (&group, Rect (33, 0, 64, 32));
	c.set_outline_width (0);
	Rectangle d (&group, Rect (33, 33, 64, 64));
	d.set_outline_width (0);
	boost::optional<Rect> bbox = group.bounding_box ();
	
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 64);
	CPPUNIT_ASSERT (bbox.get().y1 == 64);

	/* check that adding an item resets the bbox */
	
	Rectangle e (&group, Rect (64, 64, 128, 128));
	bbox = group.bounding_box ();

	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 128.5);
	CPPUNIT_ASSERT (bbox.get().y1 == 128.5);
}

void
GroupTest::null_bounding_box ()
{
	/* Check that a group containing only items with
	   no bounding box itself has no bounding box.
	*/

	ImageCanvas canvas;
	Group group (&canvas);

	Group empty (&group);

	boost::optional<Rect> bbox = empty.bounding_box ();
	CPPUNIT_ASSERT (!bbox.is_initialized ());
}

void
GroupTest::layers ()
{
	ImageCanvas canvas;
	Group group (&canvas);
	Rectangle a (&group, Rect (0, 0, 32, 32));
	Rectangle b (&group, Rect (0, 0, 32, 32));
	Rectangle c (&group, Rect (0, 0, 32, 32));
	Rectangle d (&group, Rect (0, 0, 32, 32));

	/* start: from the bottom a - b - c - d */

	a.raise_to_top ();

	list<Item*>::const_iterator i = group.items().begin();
	CPPUNIT_ASSERT (*i++ == &b);
	CPPUNIT_ASSERT (*i++ == &c);
	CPPUNIT_ASSERT (*i++ == &d);
	CPPUNIT_ASSERT (*i++ == &a);

	a.lower_to_bottom ();

	i = group.items().begin();
	CPPUNIT_ASSERT (*i++ == &a);
	CPPUNIT_ASSERT (*i++ == &b);
	CPPUNIT_ASSERT (*i++ == &c);
	CPPUNIT_ASSERT (*i++ == &d);

	a.raise (2);

	i = group.items().begin();
	CPPUNIT_ASSERT (*i++ == &b);
	CPPUNIT_ASSERT (*i++ == &c);
	CPPUNIT_ASSERT (*i++ == &a);
	CPPUNIT_ASSERT (*i++ == &d);

	a.raise (4);

	i = group.items().begin();
	CPPUNIT_ASSERT (*i++ == &b);
	CPPUNIT_ASSERT (*i++ == &c);
	CPPUNIT_ASSERT (*i++ == &d);
	CPPUNIT_ASSERT (*i++ == &a);
}

void
GroupTest::children_changing ()
{
	ImageCanvas canvas;
	Group group (&canvas);

	Rectangle a (&group, Rect (0, 0, 32, 32));
	a.set_outline_width (0);

	boost::optional<Rect> bbox = group.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 32);
	CPPUNIT_ASSERT (bbox.get().y1 == 32);

	a.set (Rect (0, 0, 48, 48));
	bbox = group.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 48);
	CPPUNIT_ASSERT (bbox.get().y1 == 48);

	a.set_outline_width (1);
	bbox = group.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == -0.5);
	CPPUNIT_ASSERT (bbox.get().y0 == -0.5);
	CPPUNIT_ASSERT (bbox.get().x1 == 48.5);
	CPPUNIT_ASSERT (bbox.get().y1 == 48.5);
}

void
GroupTest::grandchildren_changing ()
{
	ImageCanvas canvas;
	
	Group A (&canvas);

	Group B (&A);

	Rectangle a (&B, Rect (0, 0, 32, 32));
	a.set_outline_width (0);

	boost::optional<Rect> bbox = A.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 32);
	CPPUNIT_ASSERT (bbox.get().y1 == 32);

	bbox = B.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 32);
	CPPUNIT_ASSERT (bbox.get().y1 == 32);

	a.set (Rect (0, 0, 48, 48));

	bbox = A.bounding_box ();	
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 48);
	CPPUNIT_ASSERT (bbox.get().y1 == 48);

	bbox = B.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 48);
	CPPUNIT_ASSERT (bbox.get().y1 == 48);
}
