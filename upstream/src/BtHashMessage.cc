#include "BtHashMessage.h"

#include <cstring>
#include "bittorrent_helper.h"
#include "BtMessageDispatcher.h"
#include "fmt.h"
#include "DlAbortEx.h"

namespace aria2 {

const char BtHashRequestMessage::NAME[] = "hash request";
const char BtHashesMessage::NAME[] = "hashes";
const char BtHashRejectMessage::NAME[] = "hash reject";

namespace {
const size_t HASH_HEADER_PAYLOAD_LENGTH = 49;

void parseHashHeader(const unsigned char* data, size_t dataLength, uint8_t id,
                     const char* name, std::string& root, uint32_t& baseLayer,
                     uint32_t& index, uint32_t& length, uint32_t& proofLayers,
                     bool allowHashes)
{
  if (dataLength < HASH_HEADER_PAYLOAD_LENGTH ||
      (!allowHashes && dataLength != HASH_HEADER_PAYLOAD_LENGTH) ||
      (allowHashes && (dataLength - HASH_HEADER_PAYLOAD_LENGTH) % 32 != 0)) {
    throw DL_ABORT_EX(fmt("Invalid %s message length.", name));
  }
  bittorrent::assertID(id, data, name);
  root.assign(reinterpret_cast<const char*>(data + 1), 32);
  baseLayer = bittorrent::getIntParam(data, 33);
  index = bittorrent::getIntParam(data, 37);
  length = bittorrent::getIntParam(data, 41);
  proofLayers = bittorrent::getIntParam(data, 45);
}

std::vector<unsigned char> createHashMessage(uint8_t id, const std::string& root,
                                              uint32_t baseLayer, uint32_t index,
                                              uint32_t length,
                                              uint32_t proofLayers,
                                              const std::string& hashes)
{
  if (root.size() != 32 || hashes.size() % 32 != 0) {
    throw DL_ABORT_EX("Invalid BEP52 hash message data.");
  }
  const size_t payloadLength = HASH_HEADER_PAYLOAD_LENGTH + hashes.size();
  std::vector<unsigned char> msg(payloadLength + 4);
  bittorrent::createPeerMessageString(msg.data(), msg.size(), payloadLength, id);
  std::memcpy(msg.data() + 5, root.data(), 32);
  bittorrent::setIntParam(msg.data() + 37, baseLayer);
  bittorrent::setIntParam(msg.data() + 41, index);
  bittorrent::setIntParam(msg.data() + 45, length);
  bittorrent::setIntParam(msg.data() + 49, proofLayers);
  if (!hashes.empty()) {
    std::memcpy(msg.data() + 53, hashes.data(), hashes.size());
  }
  return msg;
}
} // namespace

BtHashRequestMessage::BtHashRequestMessage(const std::string& root,
                                           uint32_t baseLayer, uint32_t index,
                                           uint32_t length,
                                           uint32_t proofLayers)
    : SimpleBtMessage(ID, NAME), piecesRoot_(root), baseLayer_(baseLayer),
      index_(index), length_(length), proofLayers_(proofLayers) {}

std::unique_ptr<BtHashRequestMessage>
BtHashRequestMessage::create(const unsigned char* data, size_t dataLength)
{
  std::string root; uint32_t baseLayer, index, length, proofLayers;
  parseHashHeader(data, dataLength, ID, NAME, root, baseLayer, index, length,
                  proofLayers, false);
  if (length < 2 || (length & (length - 1)) != 0 || index % length != 0 ||
      length > 512) {
    throw DL_ABORT_EX("Invalid BEP52 hash request range.");
  }
  return make_unique<BtHashRequestMessage>(root, baseLayer, index, length,
                                           proofLayers);
}

std::vector<unsigned char> BtHashRequestMessage::createMessage()
{
  return createHashMessage(ID, piecesRoot_, baseLayer_, index_, length_,
                           proofLayers_, "");
}

std::string BtHashRequestMessage::toString() const
{
  return fmt("%s base=%u index=%u length=%u proof=%u", NAME, baseLayer_, index_,
             length_, proofLayers_);
}

void BtHashRequestMessage::doReceivedAction()
{
  // Conservative first implementation: acknowledge every syntactically valid
  // request with the mandatory protocol-level reject rather than disconnecting.
  auto reject = make_unique<BtHashRejectMessage>(
      piecesRoot_, baseLayer_, index_, length_, proofLayers_);
  reject->setCuid(getCuid());
  reject->setPeer(getPeer());
  reject->setPieceStorage(getPieceStorage());
  reject->setBtMessageDispatcher(getBtMessageDispatcher());
  reject->setBtMessageFactory(getBtMessageFactory());
  reject->setBtRequestFactory(getBtRequestFactory());
  reject->setPeerConnection(getPeerConnection());
  getBtMessageDispatcher()->addMessageToQueue(std::move(reject));
}

BtHashesMessage::BtHashesMessage(const std::string& root, uint32_t baseLayer,
                                 uint32_t index, uint32_t length,
                                 uint32_t proofLayers,
                                 const std::string& hashes)
    : SimpleBtMessage(ID, NAME), piecesRoot_(root), hashes_(hashes),
      baseLayer_(baseLayer), index_(index), length_(length),
      proofLayers_(proofLayers) {}

std::unique_ptr<BtHashesMessage>
BtHashesMessage::create(const unsigned char* data, size_t dataLength)
{
  std::string root; uint32_t baseLayer, index, length, proofLayers;
  parseHashHeader(data, dataLength, ID, NAME, root, baseLayer, index, length,
                  proofLayers, true);
  std::string hashes(reinterpret_cast<const char*>(data + 49), dataLength - 49);
  return make_unique<BtHashesMessage>(root, baseLayer, index, length, proofLayers,
                                      hashes);
}

std::vector<unsigned char> BtHashesMessage::createMessage()
{
  return createHashMessage(ID, piecesRoot_, baseLayer_, index_, length_,
                           proofLayers_, hashes_);
}

std::string BtHashesMessage::toString() const
{
  return fmt("%s base=%u index=%u length=%u proof=%u hashes=%lu", NAME,
             baseLayer_, index_, length_, proofLayers_,
             static_cast<unsigned long>(hashes_.size() / 32));
}

BtHashRejectMessage::BtHashRejectMessage(const std::string& root,
                                         uint32_t baseLayer, uint32_t index,
                                         uint32_t length,
                                         uint32_t proofLayers)
    : SimpleBtMessage(ID, NAME), piecesRoot_(root), baseLayer_(baseLayer),
      index_(index), length_(length), proofLayers_(proofLayers) {}

std::unique_ptr<BtHashRejectMessage>
BtHashRejectMessage::create(const unsigned char* data, size_t dataLength)
{
  std::string root; uint32_t baseLayer, index, length, proofLayers;
  parseHashHeader(data, dataLength, ID, NAME, root, baseLayer, index, length,
                  proofLayers, false);
  return make_unique<BtHashRejectMessage>(root, baseLayer, index, length,
                                          proofLayers);
}

std::vector<unsigned char> BtHashRejectMessage::createMessage()
{
  return createHashMessage(ID, piecesRoot_, baseLayer_, index_, length_,
                           proofLayers_, "");
}

std::string BtHashRejectMessage::toString() const
{
  return fmt("%s base=%u index=%u length=%u proof=%u", NAME, baseLayer_, index_,
             length_, proofLayers_);
}

} // namespace aria2
