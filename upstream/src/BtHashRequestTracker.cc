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
#include "BtHashRequestTracker.h"

#include <algorithm>

#include "Logger.h"
#include "LogFactory.h"
#include "fmt.h"
#include "util.h"

namespace aria2 {

BtHashRequestTracker::BtHashRequestTracker() {}

void BtHashRequestTracker::add(const std::string& piecesRoot)
{
  trackedRequests_.push_back(RequestEntry(piecesRoot));
}

bool BtHashRequestTracker::tracks(const std::string& piecesRoot) const
{
  return std::find(trackedRequests_.begin(), trackedRequests_.end(),
                   RequestEntry(piecesRoot)) != trackedRequests_.end();
}

void BtHashRequestTracker::remove(const std::string& piecesRoot)
{
  auto i = std::find(trackedRequests_.begin(), trackedRequests_.end(),
                     RequestEntry(piecesRoot));
  if (i != trackedRequests_.end()) {
    trackedRequests_.erase(i);
  }
}

namespace {
constexpr auto TIMEOUT = 20_s;
} // namespace

std::vector<std::string> BtHashRequestTracker::removeTimeoutEntry()
{
  std::vector<std::string> roots;
  trackedRequests_.erase(
      std::remove_if(std::begin(trackedRequests_), std::end(trackedRequests_),
                     [&roots](const RequestEntry& ent) {
                       if (ent.elapsed(TIMEOUT)) {
                         A2_LOG_DEBUG("BEP52 hash request timeout. "
                                      "piecesRoot(hex)=" +
                                     util::toHex(ent.piecesRoot_));
                         roots.push_back(ent.piecesRoot_);
                         return true;
                       }
                       return false;
                     }),
      std::end(trackedRequests_));
  return roots;
}

void BtHashRequestTracker::markRejected(const std::string& piecesRoot)
{
  rejectedByPeer_.insert(piecesRoot);
}

bool BtHashRequestTracker::isRejectedByPeer(const std::string& piecesRoot) const
{
  return rejectedByPeer_.count(piecesRoot) != 0;
}

} // namespace aria2
