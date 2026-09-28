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
#ifndef D_ED2K_SHARED_FILE_H
#define D_ED2K_SHARED_FILE_H

#include "common.h"

#include <string>
#include <vector>

namespace aria2 {

namespace ed2k {

struct PartRange;
class SharedSource;

std::string createSharedFileNameAnswerPayload(const SharedSource& source);
std::string createSharedFileStatusPayload(const SharedSource& source);
bool createSharedFileHashSetPayload(std::string& payload,
                                    const SharedSource& source);
bool createSharedFilePartPayload(std::string& payload,
                                 const SharedSource& source,
                                 const PartRange& range,
                                 bool use64BitOffsets);
bool createSharedFileAichAnswerPayload(std::string& payload,
                                       const SharedSource& source,
                                       uint16_t partIndex,
                                       const std::string& rootHash);
bool parsePartRequestPayload(std::vector<PartRange>& ranges,
                             std::string& fileHash,
                             const std::string& payload,
                             bool use64BitOffsets);

} // namespace ed2k

} // namespace aria2

#endif // D_ED2K_SHARED_FILE_H
