#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TypesTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (TypesTest);
	CPPUNIT_TEST (intersect);
	CPPUNIT_TEST_SUITE_END ();

public:
	void intersect ();
};

	
