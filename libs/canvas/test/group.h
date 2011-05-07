#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class GroupTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (GroupTest);
	CPPUNIT_TEST (bbox);
	CPPUNIT_TEST (null_bbox);
	CPPUNIT_TEST (layers);
	CPPUNIT_TEST (children_changing);
	CPPUNIT_TEST (grandchildren_changing);
	CPPUNIT_TEST (add_items_at_point);
	CPPUNIT_TEST (torture_add_items_at_point);
	CPPUNIT_TEST_SUITE_END ();

public:
	void bbox ();
	void null_bbox ();
	void layers ();
	void children_changing ();
	void grandchildren_changing ();
	void add_items_at_point ();
	void torture_add_items_at_point ();
};
