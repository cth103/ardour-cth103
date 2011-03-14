#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class LookupTableTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (LookupTableTest);
	CPPUNIT_TEST (build);
	CPPUNIT_TEST_SUITE_END ();

public:
	void build ();
};



