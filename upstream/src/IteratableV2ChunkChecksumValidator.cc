/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2026 aria2-ultra contributors
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 *
 * In addition, as a special exception, the copyright holders give
 * permission to link the code of portions of this program with the
 * OpenSSL library under certain conditions as described in each
 * individual source file, and distribute linked combinations
 * including the two.
 * You must obey the GNU General Public License in all respects
 * for all of the code used other than OpenSSL.  If you modify
 * file(s) with this exception, you may extend this exception to your
 * version of the file(s), but you are not obligated to do so.  If you
 * do not wish to do so, delete this exception statement from your
 * version.  If you delete this exception statement from all source
 * files in the program, then also delete it here.
 */
/* copyright --> */
#include "IteratableV2ChunkChecksumValidator.h"

#include <array>
#include <cinttypes>

#include "a2functional.h"
#include "bittorrent_helper.h"
#include "DiskAdaptor.h"
#include "DlAbortEx.h"
#include "DownloadContext.h"
#include "PieceStorage.h"
#include "BitfieldMan.h"
#include "LogFactory.h"
#include "Logger.h"
#include "message.h"
#include "RecoverableException.h"
#include "fmt.h"

namespace aria2 {

IteratableV2ChunkChecksumValidator::IteratableV2ChunkChecksumValidator(
    const std::shared_ptr<DownloadContext>& dctx,
    const std::shared_ptr<PieceStorage>& pieceStorage)
    : dctx_(dctx),
      pieceStorage_(pieceStorage),
      bitfield_(make_unique<BitfieldMan>(dctx_->getPieceLength(),
                                         dctx_->getTotalLength())),
      currentIndex_(0)
{
}

IteratableV2ChunkChecksumValidator::~IteratableV2ChunkChecksumValidator() =
    default;

void IteratableV2ChunkChecksumValidator::init()
{
  bitfield_->clearAllBit();
  currentIndex_ = 0;
}

std::string IteratableV2ChunkChecksumValidator::readPiece(int64_t offset,
                                                          size_t length)
{
  std::string data;
  data.resize(length);
  std::array<unsigned char, 4_k> buf;
  size_t got = 0;
  while (got < length) {
    size_t want = std::min(buf.size(), length - got);
    size_t r = pieceStorage_->getDiskAdaptor()->readDataDropCache(
        buf.data(), want, offset + got);
    if (r == 0) {
      throw DL_ABORT_EX(
          fmt(EX_FILE_READ, dctx_->getBasePath().c_str(), "data is too short"));
    }
    std::copy(buf.begin(), buf.begin() + r, data.begin() + got);
    got += r;
  }
  return data;
}

void IteratableV2ChunkChecksumValidator::validateChunk()
{
  if (finished()) {
    return;
  }
  try {
    const int64_t offset = getCurrentOffset();
    size_t length;
    if (currentIndex_ + 1 == dctx_->getNumPieces()) {
      length = static_cast<size_t>(dctx_->getTotalLength() - offset);
    }
    else {
      length = dctx_->getPieceLength();
    }
    const auto torrentAttrs = bittorrent::getTorrentAttrs(dctx_);
    const std::string data = readPiece(offset, length);
    if (bittorrent::verifyV2PieceByGlobalIndex(
            torrentAttrs, dctx_->getFileEntries(), currentIndex_,
            dctx_->getPieceLength(), data)) {
      bitfield_->setBit(currentIndex_);
    }
    else {
      A2_LOG_INFO(fmt("Invalid Merkle hash for piece index=%lu offset=%" PRId64,
                      static_cast<unsigned long>(currentIndex_), offset));
      bitfield_->unsetBit(currentIndex_);
    }
  }
  catch (RecoverableException& ex) {
    A2_LOG_DEBUG_EX(
        fmt("Caught exception while validating v2 piece index=%lu."
            " Some part of file may be missing. Continue operation.",
            static_cast<unsigned long>(currentIndex_)),
        ex);
    bitfield_->unsetBit(currentIndex_);
  }
  ++currentIndex_;
  if (finished()) {
    pieceStorage_->setBitfield(bitfield_->getBitfield(),
                               bitfield_->getBitfieldLength());
  }
}

bool IteratableV2ChunkChecksumValidator::finished() const
{
  return currentIndex_ >= dctx_->getNumPieces();
}

int64_t IteratableV2ChunkChecksumValidator::getCurrentOffset() const
{
  return static_cast<int64_t>(currentIndex_) * dctx_->getPieceLength();
}

int64_t IteratableV2ChunkChecksumValidator::getTotalLength() const
{
  return dctx_->getTotalLength();
}

} // namespace aria2
