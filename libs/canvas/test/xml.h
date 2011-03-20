#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class XMLTest : public CppUnit::TestFixture
{
	CPPUNIT_TEST_SUITE (XMLTest);
	CPPUNIT_TEST (get);
	CPPUNIT_TEST_SUITE_END ();

public:
	void get ();

private:	
	void check (std::string const &);
};
