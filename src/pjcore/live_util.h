// JSON [de]serialization for protobuf + embedded HTTP server and client in C++.
// Copyright (C) 2014 http://protojson.com/
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

#ifndef PJCORE_LIVE_UTIL_H_
#define PJCORE_LIVE_UTIL_H_

#include "pjcore/third_party/chromium/callback.h"
#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/live.pb.h"

namespace pjcore {

const size_t kDefaultLiveContentTruncateSize = 32;

void CaptureLiveContentOut(StringPiece content, LiveContent* live,
                           size_t truncate_size = 0);

void CaptureLiveCallbackOut(const internal::CallbackBase& callback,
                            LiveCallback* live);

template <typename List>
void CaptureLiveListPtrOut(const List& list,
                           google::protobuf::RepeatedField<uint64_t>* live) {
  for (typename List::const_iterator it = list.begin(); it != list.end();
       ++it) {
    live->Add(reinterpret_cast<uint64_t>(*it));
  }
}

}  // namespace pjcore

#endif  // PJCORE_LIVE_UTIL_H_
