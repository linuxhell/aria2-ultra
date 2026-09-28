#include "IteratableV2ChunkChecksumValidator.h"

#include <fstream>

#include <cppunit/extensions/HelperMacros.h>

#include "bittorrent_helper.h"
#include "DownloadContext.h"
#include "DefaultPieceStorage.h"
#include "Option.h"
#include "DiskAdaptor.h"
#include "FileEntry.h"

namespace aria2 {

class IteratableV2ChunkChecksumValidatorTest : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(IteratableV2ChunkChecksumValidatorTest);
  CPPUNIT_TEST(testValidate);
  CPPUNIT_TEST(testValidate_corrupted);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void testValidate();
  void testValidate_corrupted();
};

CPPUNIT_TEST_SUITE_REGISTRATION(IteratableV2ChunkChecksumValidatorTest);

namespace {
// One file, two BEP 52 pieces: a full 16 KiB piece of 'a', and a 1-byte
// final piece "b". Same shape (and the same Merkle math) as
// BittorrentHelperTest::testV2Merkle, which already proves these
// verifyV2Piece() calls succeed against hand-computed roots.
std::shared_ptr<DownloadContext> setupV2Context(const std::string& path,
                                                const std::string& piece1)
{
  const std::string piece0(16384, 'a');
  {
    std::ofstream out(path, std::ios::binary);
    out << piece0 << piece1;
  }
  const int64_t pieceLength = 16384;
  const int64_t totalLength = pieceLength + static_cast<int64_t>(piece1.size());
  auto dctx = std::make_shared<DownloadContext>(pieceLength, totalLength, path);

  const auto layer = bittorrent::computeV2MerkleRoot(piece0, 1) +
                     bittorrent::computeV2MerkleRoot(std::string(1, 'b'), 1);
  const auto root =
      bittorrent::computeV2MerkleRoot(piece0 + std::string(1, 'b'), 2);

  auto attrs = std::make_shared<TorrentAttribute>();
  attrs->metaVersion = 2;
  attrs->infoHashV2.assign(32, '\1');
  TorrentAttribute::V2FileEntry file;
  file.path = {"v2chunk.bin"};
  file.length = totalLength;
  file.piecesRoot = root;
  attrs->v2FileEntries.push_back(file);
  attrs->pieceLayers.emplace(root, layer);
  dctx->setAttribute(CTX_ATTR_BT, attrs);
  return dctx;
}
} // namespace

void IteratableV2ChunkChecksumValidatorTest::testValidate()
{
  Option option;
  auto dctx = setupV2Context(
      A2_TEST_OUT_DIR "/aria2_IteratableV2ChunkChecksumValidatorTest_ok", "b");
  auto ps = std::make_shared<DefaultPieceStorage>(dctx, &option);
  ps->initStorage();
  ps->getDiskAdaptor()->enableReadOnly();
  ps->getDiskAdaptor()->openFile();

  IteratableV2ChunkChecksumValidator validator(dctx, ps);
  validator.init();

  validator.validateChunk();
  CPPUNIT_ASSERT(!validator.finished());
  validator.validateChunk();
  CPPUNIT_ASSERT(validator.finished());
  CPPUNIT_ASSERT(ps->downloadFinished());
  CPPUNIT_ASSERT(ps->hasPiece(0));
  CPPUNIT_ASSERT(ps->hasPiece(1));
}

void IteratableV2ChunkChecksumValidatorTest::testValidate_corrupted()
{
  Option option;
  // On-disk last piece is "c", but the Merkle root/layer we hand the
  // validator were computed for "b": piece 1 must fail verification
  // while piece 0 (untouched) still passes.
  auto path = A2_TEST_OUT_DIR
      "/aria2_IteratableV2ChunkChecksumValidatorTest_corrupted";
  auto dctx = setupV2Context(path, "b");
  {
    std::fstream out(path, std::ios::binary | std::ios::in | std::ios::out);
    out.seekp(16384);
    out << "c";
  }
  auto ps = std::make_shared<DefaultPieceStorage>(dctx, &option);
  ps->initStorage();
  ps->getDiskAdaptor()->enableReadOnly();
  ps->getDiskAdaptor()->openFile();

  IteratableV2ChunkChecksumValidator validator(dctx, ps);
  validator.init();
  while (!validator.finished()) {
    validator.validateChunk();
  }
  CPPUNIT_ASSERT(ps->hasPiece(0));
  CPPUNIT_ASSERT(!ps->hasPiece(1));
}

} // namespace aria2
