#include "InitiateConnectionCommand.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class InitiateConnectionCommandTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(InitiateConnectionCommandTest);
  CPPUNIT_TEST(testSelectDistributedAddress_roundRobinsByCuid);
  CPPUNIT_TEST(testSelectDistributedAddress_wrapsAroundModulo);
  CPPUNIT_TEST(testSelectDistributedAddress_singleAddressAlwaysChosen);
  CPPUNIT_TEST_SUITE_END();

public:
  void testSelectDistributedAddress_roundRobinsByCuid();
  void testSelectDistributedAddress_wrapsAroundModulo();
  void testSelectDistributedAddress_singleAddressAlwaysChosen();
};

CPPUNIT_TEST_SUITE_REGISTRATION(InitiateConnectionCommandTest);

// Four parallel connections for the same segmented download (consecutive
// CUIDs, as DownloadEngine::newCUID() hands out) must land on four
// different resolved addresses when there are exactly four to choose from -
// this is the whole point of the feature: spreading --split connections
// across every A/AAAA record instead of pinning them all to the first one.
void InitiateConnectionCommandTest::
    testSelectDistributedAddress_roundRobinsByCuid()
{
  std::vector<std::string> addrs;
  addrs.push_back("203.0.113.1");
  addrs.push_back("203.0.113.2");
  addrs.push_back("203.0.113.3");
  addrs.push_back("203.0.113.4");
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.1"),
                       selectDistributedAddress(addrs, 100));
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.2"),
                       selectDistributedAddress(addrs, 101));
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.3"),
                       selectDistributedAddress(addrs, 102));
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.4"),
                       selectDistributedAddress(addrs, 103));
}

// More connections than addresses must wrap back to the start rather than
// going out of bounds or always landing on the same tail address.
void InitiateConnectionCommandTest::
    testSelectDistributedAddress_wrapsAroundModulo()
{
  std::vector<std::string> addrs;
  addrs.push_back("203.0.113.1");
  addrs.push_back("203.0.113.2");
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.1"),
                       selectDistributedAddress(addrs, 0));
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.2"),
                       selectDistributedAddress(addrs, 1));
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.1"),
                       selectDistributedAddress(addrs, 2));
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.2"),
                       selectDistributedAddress(addrs, 3));
}

// A hostname that only ever resolves to one address (the overwhelmingly
// common case) must always get that one address back, for any CUID.
void InitiateConnectionCommandTest::
    testSelectDistributedAddress_singleAddressAlwaysChosen()
{
  std::vector<std::string> addrs;
  addrs.push_back("203.0.113.9");
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.9"),
                       selectDistributedAddress(addrs, 0));
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.9"),
                       selectDistributedAddress(addrs, 42));
  CPPUNIT_ASSERT_EQUAL(std::string("203.0.113.9"),
                       selectDistributedAddress(addrs, 999999));
}

} // namespace aria2
