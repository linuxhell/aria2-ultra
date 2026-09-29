#include "BtHashesMessage.h"
#include "bittorrent_helper.h"
#include "DlAbortEx.h"
#include "fmt.h"

#include <algorithm>

namespace aria2 {

const char BtHashesMessage::NAME[] = "hashes";

BtHashesMessage::BtHashesMessage(const std::string& piecesRoot,
                                 uint32_t baseLayer, uint32_t index,
                                 uint32_t length, uint32_t proofLayers,
                                 const std::string& hashes)
    : SimpleBtMessage(ID, NAME),
      piecesRoot_(piecesRoot),
      baseLayer_(baseLayer),
      index_(index),
      length_(length),
      proofLayers_(proofLayers),
      hashes_(hashes)
{
}

std::unique_ptr<BtHashesMessage>
BtHashesMessage::create(const unsigned char* data, size_t dataLength)
{
  if (dataLength < 49 || (dataLength - 49) % 32 != 0) {
    throw DL_ABORT_EX("Invalid BEP 52 hashes payload length.");
  }
  bittorrent::assertID(ID, data, NAME);
  auto msg = make_unique<BtHashesMessage>(
      std::string(reinterpret_cast<const char*>(data + 1), 32),
      bittorrent::getIntParam(data, 33), bittorrent::getIntParam(data, 37),
      bittorrent::getIntParam(data, 41), bittorrent::getIntParam(data, 45),
      std::string(reinterpret_cast<const char*>(data + 49), dataLength - 49));
  msg->validate();
  return msg;
}

void BtHashesMessage::validate()
{
  if (piecesRoot_.size() != 32 || length_ < 2 ||
      (length_ & (length_ - 1)) != 0 || index_ % length_ != 0 ||
      hashes_.size() % 32 != 0 ||
      hashes_.size() < static_cast<size_t>(length_) * 32) {
    throw DL_ABORT_EX("Invalid BEP 52 hashes message.");
  }
}

std::vector<unsigned char> BtHashesMessage::createMessage()
{
  validate();
  const size_t payloadLength = 49 + hashes_.size();
  std::vector<unsigned char> msg(4 + payloadLength);
  bittorrent::createPeerMessageString(msg.data(), msg.size(), payloadLength, ID);
  std::copy(piecesRoot_.begin(), piecesRoot_.end(), msg.begin() + 5);
  bittorrent::setIntParam(&msg[37], baseLayer_);
  bittorrent::setIntParam(&msg[41], index_);
  bittorrent::setIntParam(&msg[45], length_);
  bittorrent::setIntParam(&msg[49], proofLayers_);
  std::copy(hashes_.begin(), hashes_.end(), msg.begin() + 53);
  return msg;
}

std::string BtHashesMessage::toString() const
{
  return fmt("%s base=%u index=%u length=%u proof=%u hashes=%lu", NAME,
             baseLayer_, index_, length_, proofLayers_,
             static_cast<unsigned long>(hashes_.size() / 32));
}

} // namespace aria2
