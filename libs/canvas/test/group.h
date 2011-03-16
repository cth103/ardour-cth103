#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class GroupTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (GroupTest);
	CPPUNIT_TEST (bounding_box);
	CPPUNIT_TEST (layers);
	CPPUNIT_TEST_SUITE_END ();

public:
	void bounding_box ();
	void layers ();
};