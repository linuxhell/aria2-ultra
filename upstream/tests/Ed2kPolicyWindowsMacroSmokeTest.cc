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

#define CALLBACK __attribute__((__stdcall__))
#include "ed2k_policy.h"
#undef CALLBACK

namespace aria2 {
namespace {

constexpr auto peerActionTypeCallbackCompileSmoke =
    ed2k::PeerActionType::REQUEST_CALLBACK;

} // namespace
} // namespace aria2
