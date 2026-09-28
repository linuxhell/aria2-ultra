/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2026 aria2-next contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */
/* copyright --> */
#ifndef D_ED2K_SHARE_INDEX_H
#define D_ED2K_SHARE_INDEX_H

#include "common.h"

#include <memory>
#include <string>
#include <vector>

namespace aria2 {

class DownloadContext;
class PieceStorage;
class RequestGroupMan;

namespace ed2k {

class SharedSource {
public:
  virtual ~SharedSource() = default;

  virtual const std::string& hash() const = 0;
  virtual const std::string& aichRootHash() const = 0;
  virtual const std::vector<std::string>& pieceHashes() const = 0;
  virtual const std::string& name() const = 0;
  virtual int64_t size() const = 0;
  virtual bool complete() const = 0;
  virtual std::vector<bool> bitfield() const = 0;
  virtual bool readRange(std::string& data, int64_t begin,
                         int64_t end) const = 0;
  virtual void recordUpload(size_t bytes) = 0;
};

std::unique_ptr<SharedSource>
createActiveSharedSource(DownloadContext* dctx, PieceStorage* pieceStorage,
                         const std::string& path);

std::unique_ptr<SharedSource>
findSharedSource(RequestGroupMan* rgman, const std::string& hash);

std::vector<std::shared_ptr<SharedSource>>
listSharedSources(RequestGroupMan* rgman);

bool hasSharedSources(RequestGroupMan* rgman);

bool createOfferFilesPayload(
    std::string& payload,
    const std::vector<std::shared_ptr<SharedSource>>& sources,
    bool supportsLargeFiles, size_t limit, uint32_t clientId,
    uint16_t clientPort);

} // namespace ed2k

} // namespace aria2

#endif // D_ED2K_SHARE_INDEX_H
