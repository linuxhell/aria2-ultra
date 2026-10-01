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
#ifndef D_BT_HASH_REQUEST_TRACKER_H
#define D_BT_HASH_REQUEST_TRACKER_H

#include "common.h"

#include <string>
#include <vector>
#include <set>

#include "TimerA2.h"
#include "wallclock.h"

namespace aria2 {

// Tracks outstanding BEP 52 BtHashRequestMessage(s) sent to a single peer
// connection, keyed by the requested file's piecesRoot. One connection only
// ever has one outstanding request per piecesRoot (we always request the
// whole piece layer in a single message).
class BtHashRequestTracker {
public:
  struct RequestEntry {
    std::string piecesRoot_;
    Timer dispatchedTime_;

    RequestEntry(const std::string& piecesRoot) : piecesRoot_(piecesRoot) {}

    bool elapsed(const std::chrono::seconds t) const
    {
      return dispatchedTime_.difference(global::wallclock()) >= t;
    }

    bool operator==(const RequestEntry& e) const
    {
      return piecesRoot_ == e.piecesRoot_;
    }
  };

private:
  std::vector<RequestEntry> trackedRequests_;

  // piecesRoot values this connection's peer has explicitly rejected
  // (BtHashRejectMessage): do not ask this same connection again.
  std::set<std::string> rejectedByPeer_;

public:
  BtHashRequestTracker();

  void add(const std::string& piecesRoot);

  bool tracks(const std::string& piecesRoot) const;

  void remove(const std::string& piecesRoot);

  // Removes and returns piecesRoot values that timed out waiting for a
  // Hashes/HashReject reply; caller may retry them with another peer.
  std::vector<std::string> removeTimeoutEntry();

  void markRejected(const std::string& piecesRoot);

  bool isRejectedByPeer(const std::string& piecesRoot) const;

  size_t count() const { return trackedRequests_.size(); }
};

} // namespace aria2

#endif // D_BT_HASH_REQUEST_TRACKER_H
