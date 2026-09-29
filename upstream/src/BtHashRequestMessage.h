#ifndef D_BT_HASH_REQUEST_MESSAGE_H
#define D_BT_HASH_REQUEST_MESSAGE_H

#include "SimpleBtMessage.h"
#include <string>

namespace aria2 {

class BtHashRequestMessage : public SimpleBtMessage {
private:
  std::string piecesRoot_;
  uint32_t baseLayer_;
  uint32_t index_;
  uint32_t length_;
  uint32_t proofLayers_;

public:
  static const uint8_t ID = 21;
  static const char NAME[];

  BtHashRequestMessage(const std::string& piecesRoot = std::string(),
                       uint32_t baseLayer = 0, uint32_t index = 0,
                       uint32_t length = 0, uint32_t proofLayers = 0);

  static std::unique_ptr<BtHashRequestMessage>
  create(const unsigned char* data, size_t dataLength);

  const std::string& getPiecesRoot() const { return piecesRoot_; }
  uint32_t getBaseLayer() const { return baseLayer_; }
  uint32_t getIndex() const { return index_; }
  uint32_t getLength() const { return length_; }
  uint32_t getProofLayers() const { return proofLayers_; }

  virtual std::vector<unsigned char> createMessage() CXX11_OVERRIDE;
  virtual std::string toString() const CXX11_OVERRIDE;
  virtual void validate() CXX11_OVERRIDE;
};

} // namespace aria2
#endif
