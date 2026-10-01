#ifndef D_BT_HASH_MESSAGE_H
#define D_BT_HASH_MESSAGE_H

#include "SimpleBtMessage.h"
#include <string>

namespace aria2 {

class DownloadContext;
class BtHashRequestTracker;

class BtHashRequestMessage : public SimpleBtMessage {
public:
  static const uint8_t ID = 21;
  static const char NAME[];
  BtHashRequestMessage(const std::string& piecesRoot = "", uint32_t baseLayer = 0,
                       uint32_t index = 0, uint32_t length = 0,
                       uint32_t proofLayers = 0);
  static std::unique_ptr<BtHashRequestMessage> create(const unsigned char* data,
                                                       size_t dataLength);
  std::vector<unsigned char> createMessage() CXX11_OVERRIDE;
  std::string toString() const CXX11_OVERRIDE;
  void doReceivedAction() CXX11_OVERRIDE;
  const std::string& getPiecesRoot() const { return piecesRoot_; }
  uint32_t getBaseLayer() const { return baseLayer_; }
  uint32_t getIndex() const { return index_; }
  uint32_t getLength() const { return length_; }
  uint32_t getProofLayers() const { return proofLayers_; }
  void setDownloadContext(DownloadContext* dctx) { downloadContext_ = dctx; }
private:
  std::string piecesRoot_;
  uint32_t baseLayer_, index_, length_, proofLayers_;
  DownloadContext* downloadContext_ = nullptr;
};

class BtHashesMessage : public SimpleBtMessage {
public:
  static const uint8_t ID = 22;
  static const char NAME[];
  BtHashesMessage(const std::string& piecesRoot = "", uint32_t baseLayer = 0,
                  uint32_t index = 0, uint32_t length = 0,
                  uint32_t proofLayers = 0, const std::string& hashes = "");
  static std::unique_ptr<BtHashesMessage> create(const unsigned char* data,
                                                  size_t dataLength);
  std::vector<unsigned char> createMessage() CXX11_OVERRIDE;
  std::string toString() const CXX11_OVERRIDE;
  void doReceivedAction() CXX11_OVERRIDE;
  const std::string& getPiecesRoot() const { return piecesRoot_; }
  const std::string& getHashes() const { return hashes_; }
  uint32_t getBaseLayer() const { return baseLayer_; }
  uint32_t getProofLayers() const { return proofLayers_; }
  void setDownloadContext(DownloadContext* dctx) { downloadContext_ = dctx; }
  void setHashRequestTracker(BtHashRequestTracker* tracker)
  {
    hashRequestTracker_ = tracker;
  }
private:
  std::string piecesRoot_, hashes_;
  uint32_t baseLayer_, index_, length_, proofLayers_;
  DownloadContext* downloadContext_ = nullptr;
  BtHashRequestTracker* hashRequestTracker_ = nullptr;
};

class BtHashRejectMessage : public SimpleBtMessage {
public:
  static const uint8_t ID = 23;
  static const char NAME[];
  BtHashRejectMessage(const std::string& piecesRoot = "", uint32_t baseLayer = 0,
                      uint32_t index = 0, uint32_t length = 0,
                      uint32_t proofLayers = 0);
  static std::unique_ptr<BtHashRejectMessage> create(const unsigned char* data,
                                                      size_t dataLength);
  std::vector<unsigned char> createMessage() CXX11_OVERRIDE;
  std::string toString() const CXX11_OVERRIDE;
  void doReceivedAction() CXX11_OVERRIDE;
  const std::string& getPiecesRoot() const { return piecesRoot_; }
  void setHashRequestTracker(BtHashRequestTracker* tracker)
  {
    hashRequestTracker_ = tracker;
  }
private:
  std::string piecesRoot_;
  uint32_t baseLayer_, index_, length_, proofLayers_;
  BtHashRequestTracker* hashRequestTracker_ = nullptr;
};

} // namespace aria2
#endif
