#include "canvas/group.h"
#include "canvas/types.h"
#include "canvas/rectangle.h"
#include "canvas/canvas.h"
#include "group.h"

using namespace std;
using namespace ArdourCanvas;

CPPUNIT_TEST_SUITE_REGISTRATION (GroupTest);

extern Rectangle* shitcunt;

/* Do some basic checks on the group's computation of its bounding box */
void
GroupTest::bounding_box ()
{
	/* a group with 4 rectangles in it */
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
	boost::optional<Rect> bbox = group.bounding_box ();

	/* check the bounding box */
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
	CPPUNIT_ASSERT (bbox.get().x1 == 128.25);
	CPPUNIT_ASSERT (bbox.get().y1 == 128.25);
}

/* Check that a group containing only items with no bounding box itself has no bounding box */
void
GroupTest::null_bounding_box ()
{
	ImageCanvas canvas;
	RootGroup group (&canvas);

	Group empty (&group);

	boost::optional<Rect> bbox = empty.bounding_box ();
	CPPUNIT_ASSERT (!bbox.is_initialized ());
}

/* Do some basic tests on layering */
void
GroupTest::layers ()
{
	/* Set up 4 rectangles; order from the bottom is
	   a - b - c - d
	*/
	ImageCanvas canvas;
	RootGroup group (&canvas);
	Rectangle a (&group, Rect (0, 0, 32, 32));
	Rectangle b (&group, Rect (0, 0, 32, 32));
	Rectangle c (&group, Rect (0, 0, 32, 32));
	Rectangle d (&group, Rect (0, 0, 32, 32));

	/* Put a on top and check */
	a.raise_to_top ();

	list<Item*>::const_iterator i = group.items().begin();
	CPPUNIT_ASSERT (*i++ == &b);
	CPPUNIT_ASSERT (*i++ == &c);
	CPPUNIT_ASSERT (*i++ == &d);
	CPPUNIT_ASSERT (*i++ == &a);

	/* Put a on the bottom and check */
	a.lower_to_bottom ();

	i = group.items().begin();
	CPPUNIT_ASSERT (*i++ == &a);
	CPPUNIT_ASSERT (*i++ == &b);
	CPPUNIT_ASSERT (*i++ == &c);
	CPPUNIT_ASSERT (*i++ == &d);

	/* Check raise by a number of levels */
	
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

/* Check that groups notice when their children change */
void
GroupTest::children_changing ()
{
	ImageCanvas canvas;

	/* A root group with a rectangle in it */
	RootGroup group (&canvas);
	Rectangle a (&group, Rect (0, 0, 32, 32));
	a.set_outline_width (0);

	/* Check that initial bbox */
	boost::optional<Rect> bbox = group.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 32);
	CPPUNIT_ASSERT (bbox.get().y1 == 32);

	/* Change the rectangle's size and check the parent */
	a.set (Rect (0, 0, 48, 48));
	bbox = group.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == 0);
	CPPUNIT_ASSERT (bbox.get().y0 == 0);
	CPPUNIT_ASSERT (bbox.get().x1 == 48);
	CPPUNIT_ASSERT (bbox.get().y1 == 48);

	/* Change the rectangle's line width and check the parent */
	a.set_outline_width (1);
	bbox = group.bounding_box ();
	CPPUNIT_ASSERT (bbox.is_initialized ());
	CPPUNIT_ASSERT (bbox.get().x0 == -0.5);
	CPPUNIT_ASSERT (bbox.get().y0 == -0.5);
	CPPUNIT_ASSERT (bbox.get().x1 == 48.5);
	CPPUNIT_ASSERT (bbox.get().y1 == 48.5);
}

/* Check that a group notices when its grandchildren change */
void
GroupTest::grandchildren_changing ()
{
	ImageCanvas canvas;

	/* A root group A with a child group B */
	RootGroup A (&canvas);
	Group B (&A);

	/* Grandchild rectangle */
	Rectangle a (&B, Rect (0, 0, 32, 32));
	a.set_outline_width (0);

	/* Check the initial bboxes */
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

	/* Change the grandchild and check its parent and grandparent */
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

/* Basic tests on the code to find items at a particular point */
void
GroupTest::add_items_at_point ()
{
	ImageCanvas canvas;
	
	RootGroup root (&canvas);

	Group gA (&root);
	gA.set_position (Duple (128, 64));

	Group gB (&gA);
	gB.set_position (Duple (64, 32));

	/* two rectangles in the same place, rB on top of rA */
	Rectangle rA (&gB);
	rA.set_position (Duple (4, 2));
	rA.set (Rect (0, 0, 8, 4));
	Rectangle rB (&gB);
	rB.set_position (Duple (4, 2));
	rB.set (Rect (0, 0, 8, 4));

	/* rC below those two */
	Rectangle rC (&gB);
	rC.set_position (Duple (12, 6));
	rC.set (Rect (0, 0, 8, 4));

	list<Item*> items;
	root.add_items_at_point (Duple (128 + 64 + 4 + 4, 64 + 32 + 2 + 2), items);
	CPPUNIT_ASSERT (items.size() == 5);
	list<Item*>::iterator i = items.begin ();
	CPPUNIT_ASSERT (*i++ == &root);
	CPPUNIT_ASSERT (*i++ == &gA);
	CPPUNIT_ASSERT (*i++ == &gB);
	CPPUNIT_ASSERT (*i++ == &rA);
	CPPUNIT_ASSERT (*i++ == &rB);

	items.clear ();
	root.add_items_at_point (Duple (128 + 64 + 12 + 4, 64 + 32 + 6 + 2), items);
	CPPUNIT_ASSERT (items.size() == 4);
	i = items.begin ();
	CPPUNIT_ASSERT (*i++ == &root);
	CPPUNIT_ASSERT (*i++ == &gA);
	CPPUNIT_ASSERT (*i++ == &gB);
	CPPUNIT_ASSERT (*i++ == &rC);
}

static double
double_random ()
{
	return ((double) rand() / RAND_MAX);
}

/* Check the find items at point code more thoroughly */
void
GroupTest::torture_add_items_at_point ()
{
	int const n_rectangles = 10000;
	int const n_tests = 1000;
	double const rough_size = 1000;
	srand (1);

	ImageCanvas canvas;
	RootGroup group (&canvas);

	list<Item*> rectangles;

	for (int i = 0; i < n_rectangles; ++i) {
		Rectangle* r = new Rectangle (&group);
		double const x = double_random () * rough_size / 2;
		double const y = double_random () * rough_size / 2;
		double const w = double_random () * rough_size / 2;
		double const h = double_random () * rough_size / 2;
		r->set (Rect (x, y, x + w, y + h));
		rectangles.push_back (r);
	}

	for (int i = 0; i < n_tests; ++i) {
		Duple test (double_random() * rough_size, double_random() * rough_size);

		/* ask the group what's at this point */
		list<Item*> items_A;
		group.add_items_at_point (test, items_A);

		/* work it out ourselves */
		list<Item*> items_B;
		if (group.bounding_box() && group.bounding_box().get().contains (test)) {
			items_B.push_back (&group);
		}
		
		for (list<Item*>::iterator j = rectangles.begin(); j != rectangles.end(); ++j) {
			boost::optional<Rect> bbox = (*j)->bounding_box ();
			assert (bbox);
			if (bbox.get().contains (test)) {
				items_B.push_back (*j);
			}
		}

		CPPUNIT_ASSERT (items_A.size() == items_B.size());
		list<Item*>::iterator j = items_A.begin ();
		list<Item*>::iterator k = items_B.begin ();
		while (j != items_A.end ()) {
			CPPUNIT_ASSERT (*j == *k);
			++j;
			++k;
		}
	}
}
	
