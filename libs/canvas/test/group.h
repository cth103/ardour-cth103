#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class GroupTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (GroupTest);
	CPPUNIT_TEST (bounding_box);
	CPPUNIT_TEST (null_bounding_box);
	CPPUNIT_TEST (layers);
	CPPUNIT_TEST (children_changing);
	CPPUNIT_TEST (grandchildren_changing);
	CPPUNIT_TEST_SUITE_END ();

public:
	void bounding_box ();
	void null_bounding_box ();
	void layers ();
	void children_changing ();
	void grandchildren_changing ();
};
