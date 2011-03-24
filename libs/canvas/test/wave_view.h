#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class WaveViewTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (WaveViewTest);
	CPPUNIT_TEST (basics);
	CPPUNIT_TEST_SUITE_END ();

public:
	void basics ();
};


