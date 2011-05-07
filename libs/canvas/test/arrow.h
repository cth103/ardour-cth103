#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class ArrowTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (ArrowTest);
	CPPUNIT_TEST (bbox);
	CPPUNIT_TEST_SUITE_END ();

public:
	void bbox ();
};
