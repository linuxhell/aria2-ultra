#include "BtHashRequestMessage.h"
#include "BtHashesMessage.h"
#include "BtHashRejectMessage.h"

#include <cppunit/extensions/HelperMacros.h>

namespace aria2 {

class BtHashMessageTest : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(BtHashMessageTest);
  CPPUNIT_TEST(testHashRequestRoundTrip);
  CPPUNIT_TEST(testHashesRoundTrip);
  CPPUNIT_TEST(testHashRejectRoundTrip);
  CPPUNIT_TEST(testInvalidRequest);
  CPPUNIT_TEST_SUITE_END();

public:
  void testHashRequestRoundTrip()
  {
    std::string root(32, 'r');
    BtHashRequestMessage src(root, 4, 8, 8, 5);
    auto wire = src.createMessage();
    auto dst = BtHashRequestMessage::create(wire.data() + 4, wire.size() - 4);
    CPPUNIT_ASSERT_EQUAL(root, dst->getPiecesRoot());
    CPPUNIT_ASSERT_EQUAL((uint32_t)4, dst->getBaseLayer());
    CPPUNIT_ASSERT_EQUAL((uint32_t)8, dst->getIndex());
    CPPUNIT_ASSERT_EQUAL((uint32_t)8, dst->getLength());
    CPPUNIT_ASSERT_EQUAL((uint32_t)5, dst->getProofLayers());
  }

  void testHashesRoundTrip()
  {
    std::string root(32, 'p');
    std::string hashes(4 * 32, 'h');
    BtHashesMessage src(root, 3, 0, 4, 3, hashes);
    auto wire = src.createMessage();
    auto dst = BtHashesMessage::create(wire.data() + 4, wire.size() - 4);
    CPPUNIT_ASSERT_EQUAL(root, dst->getPiecesRoot());
    CPPUNIT_ASSERT_EQUAL(hashes, dst->getHashes());
    CPPUNIT_ASSERT_EQUAL((uint32_t)4, dst->getLength());
  }

  void testHashRejectRoundTrip()
  {
    std::string root(32, 'x');
    BtHashRejectMessage src(root, 0, 0, 2, 1);
    auto wire = src.createMessage();
    auto dst = BtHashRejectMessage::create(wire.data() + 4, wire.size() - 4);
    CPPUNIT_ASSERT_EQUAL(root, dst->getPiecesRoot());
    CPPUNIT_ASSERT_EQUAL((uint32_t)2, dst->getLength());
  }

  void testInvalidRequest()
  {
    try {
      BtHashRequestMessage bad(std::string(32, 'r'), 0, 1, 3, 1);
      bad.createMessage();
      CPPUNIT_FAIL("invalid BEP 52 request must throw");
    }
    catch (...) {
    }
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(BtHashMessageTest);

} // namespace aria2
