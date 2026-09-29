#include "BtHashRejectMessage.h"
#include "bittorrent_helper.h"
#include "DlAbortEx.h"
#include "fmt.h"

#include <algorithm>

namespace aria2 {

const char BtHashRejectMessage::NAME[] = "hash reject";

BtHashRejectMessage::BtHashRejectMessage(const std::string& piecesRoot,
                                         uint32_t baseLayer, uint32_t index,
                                         uint32_t length,
                                         uint32_t proofLayers)
    : SimpleBtMessage(ID, NAME),
      piecesRoot_(piecesRoot),
      baseLayer_(baseLayer),
      index_(index),
      length_(length),
      proofLayers_(proofLayers)
{
}

std::unique_ptr<BtHashRejectMessage>
BtHashRejectMessage::create(const unsigned char* data, size_t dataLength)
{
  bittorrent::assertPayloadLengthEqual(49, dataLength, NAME);
  bittorrent::assertID(ID, data, NAME);
  auto msg = make_unique<BtHashRejectMessage>(
      std::string(reinterpret_cast<const char*>(data + 1), 32),
      bittorrent::getIntParam(data, 33), bittorrent::getIntParam(data, 37),
      bittorrent::getIntParam(data, 41), bittorrent::getIntParam(data, 45));
  msg->validate();
  return msg;
}

void BtHashRejectMessage::validate()
{
  if (piecesRoot_.size() != 32 || length_ < 2 ||
      (length_ & (length_ - 1)) != 0 || index_ % length_ != 0) {
    throw DL_ABORT_EX("Invalid BEP 52 hash reject.");
  }
}

std::vector<unsigned char> BtHashRejectMessage::createMessage()
{
  validate();
  std::vector<unsigned char> msg(53);
  bittorrent::createPeerMessageString(msg.data(), msg.size(), 49, ID);
  std::copy(piecesRoot_.begin(), piecesRoot_.end(), msg.begin() + 5);
  bittorrent::setIntParam(&msg[37], baseLayer_);
  bittorrent::setIntParam(&msg[41], index_);
  bittorrent::setIntParam(&msg[45], length_);
  bittorrent::setIntParam(&msg[49], proofLayers_);
  return msg;
}

std::string BtHashRejectMessage::toString() const
{
  return fmt("%s base=%u index=%u length=%u proof=%u", NAME, baseLayer_,
             index_, length_, proofLayers_);
}

} // namespace aria2
