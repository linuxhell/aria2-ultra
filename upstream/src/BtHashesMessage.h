#ifndef D_BT_HASHES_MESSAGE_H
#define D_BT_HASHES_MESSAGE_H

#include "SimpleBtMessage.h"
#include <string>

namespace aria2 {

class BtHashesMessage : public SimpleBtMessage {
private:
  std::string piecesRoot_;
  uint32_t baseLayer_;
  uint32_t index_;
  uint32_t length_;
  uint32_t proofLayers_;
  std::string hashes_;

public:
  static const uint8_t ID = 22;
  static const char NAME[];

  BtHashesMessage(const std::string& piecesRoot = std::string(),
                  uint32_t baseLayer = 0, uint32_t index = 0,
                  uint32_t length = 0, uint32_t proofLayers = 0,
                  const std::string& hashes = std::string());

  static std::unique_ptr<BtHashesMessage>
  create(const unsigned char* data, size_t dataLength);

  const std::string& getPiecesRoot() const { return piecesRoot_; }
  uint32_t getBaseLayer() const { return baseLayer_; }
  uint32_t getIndex() const { return index_; }
  uint32_t getLength() const { return length_; }
  uint32_t getProofLayers() const { return proofLayers_; }
  const std::string& getHashes() const { return hashes_; }

  virtual std::vector<unsigned char> createMessage() CXX11_OVERRIDE;
  virtual std::string toString() const CXX11_OVERRIDE;
  virtual void validate() CXX11_OVERRIDE;
};

} // namespace aria2
#endif
