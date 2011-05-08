#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class ItemsAtPointTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (ItemsAtPointTest);
	CPPUNIT_TEST (with_transform);
	CPPUNIT_TEST_SUITE_END ();

public:
	void with_transform ();
};
