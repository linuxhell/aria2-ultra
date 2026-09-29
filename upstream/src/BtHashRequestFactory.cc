/* <!-- copyright */
/*
 * aria2 - The high speed download utility
 *
 * Copyright (C) 2009 Tatsuhiro Tsujikawa
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
#include "BtHashRequestFactory.h"

#include "DownloadContext.h"
#include "TorrentAttribute.h"
#include "BtConstants.h"
#include "bittorrent_helper.h"
#include "BtMessageFactory.h"
#include "BtHashRequestTracker.h"
#include "BtHashMessage.h"
#include "BtMessage.h"
#include "Logger.h"
#include "LogFactory.h"
#include "fmt.h"
#include "util.h"

namespace aria2 {

BtHashRequestFactory::BtHashRequestFactory()
    : dctx_{nullptr}, messageFactory_{nullptr}, tracker_{nullptr}
{
}

std::vector<std::unique_ptr<BtMessage>> BtHashRequestFactory::create(size_t num)
{
  auto msgs = std::vector<std::unique_ptr<BtMessage>>{};
  if (!dctx_ || !dctx_->hasAttribute(CTX_ATTR_BT)) {
    return msgs;
  }
  TorrentAttribute* torrentAttrs = bittorrent::getTorrentAttrs(dctx_);
  const int64_t pieceLength = dctx_->getPieceLength();
  for (const auto& file : torrentAttrs->v2FileEntries) {
    if (num == 0) {
      break;
    }
    if (file.piecesRoot.size() != 32) {
      // Zero-length file or otherwise has no separate Merkle tree.
      continue;
    }
    if (torrentAttrs->pieceLayers.find(file.piecesRoot) !=
        torrentAttrs->pieceLayers.end()) {
      // Already have it.
      continue;
    }
    size_t width = bittorrent::v2PieceLayerWidth(file.length, pieceLength);
    if (width == 0 || width > 512) {
      // Single-piece file (no layer needed) or a layer too large for a
      // single BEP 52 Hash Request message (not yet supported).
      continue;
    }
    if (tracker_->tracks(file.piecesRoot) ||
        tracker_->isRejectedByPeer(file.piecesRoot)) {
      continue;
    }
    A2_LOG_DEBUG(fmt("Requesting BEP52 piece layer for piecesRoot=%s "
                     "(%lu hashes).",
                     util::toHex(file.piecesRoot).c_str(),
                     static_cast<unsigned long>(width)));
    auto m = messageFactory_->createHashRequestMessage(
        file.piecesRoot, static_cast<uint32_t>(width));
    msgs.push_back(std::move(m));
    tracker_->add(file.piecesRoot);
    --num;
  }
  return msgs;
}

} // namespace aria2
