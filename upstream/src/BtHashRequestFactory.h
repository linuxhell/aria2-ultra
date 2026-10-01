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
#ifndef D_BT_HASH_REQUEST_FACTORY_H
#define D_BT_HASH_REQUEST_FACTORY_H

#include "common.h"

#include <vector>
#include <memory>

namespace aria2 {

class DownloadContext;
class BtMessageFactory;
class BtHashRequestTracker;
class BtMessage;

// Drives the requesting side of BEP 52 "Hash Request": for a v2/hybrid
// download, some or all files' piece layers may be missing (this happens
// for a v2-only magnet, whose info dict does not carry piece layers -
// only a .torrent file does). This factory asks the connected peer, one
// whole layer per message, for any file we still lack a layer for.
//
// Simplification: each peer connection tracks and requests independently,
// with no cross-connection deduplication. When several peers are
// connected, aria2 may ask more than one of them for the same layer at
// once; the first valid reply wins and later ones are silently ignored
// (see BtHashesMessage::doReceivedAction). This trades a little redundant
// traffic for not needing a new shared, per-download tracking structure.
class BtHashRequestFactory {
private:
  DownloadContext* dctx_;
  BtMessageFactory* messageFactory_;
  BtHashRequestTracker* tracker_;

public:
  BtHashRequestFactory();

  // Creates and returns at most num BtHashRequestMessage(s) for files
  // whose piece layer this download still needs and this connection is
  // not already waiting on (or was rejected for).
  std::vector<std::unique_ptr<BtMessage>> create(size_t num);

  void setDownloadContext(DownloadContext* dctx) { dctx_ = dctx; }

  void setBtMessageFactory(BtMessageFactory* factory)
  {
    messageFactory_ = factory;
  }

  void setHashRequestTracker(BtHashRequestTracker* tracker)
  {
    tracker_ = tracker;
  }
};

} // namespace aria2

#endif // D_BT_HASH_REQUEST_FACTORY_H
