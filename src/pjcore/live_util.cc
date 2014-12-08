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

#include "pjcore/live_util.h"

#include "pjcore/unicode.h"
#include "pjcore/string_piece_util.h"

namespace pjcore {

namespace {

typedef void (*CallbackInvokeFuncStorage)(void);

class PublicCallbackBase : public internal::CallbackBase {
 public:
  const scoped_refptr<internal::BindStateBase>& bind_state() const {
    return bind_state_;
  }

  const InvokeFuncStorage& polymorphic_invoke() const {
    return polymorphic_invoke_;
  }
};

const internal::BindStateBase* GetCallbackBindStateBase(
    const internal::CallbackBase& callback) {
  const PublicCallbackBase& public_callback =
      static_cast<const PublicCallbackBase&>(callback);

  return public_callback.bind_state().get();
}

CallbackInvokeFuncStorage GetCallbackPolymorphicInvoke(
    const internal::CallbackBase& callback) {
  const PublicCallbackBase& public_callback =
      static_cast<const PublicCallbackBase&>(callback);

  return public_callback.polymorphic_invoke();
}

}  // unnamed namespace

void CaptureLiveContentOut(StringPiece content, LiveContent* live,
                           size_t truncate_size) {
  PJCORE_CHECK(live);
  live->Clear();

  if (truncate_size == 0) {
    truncate_size = kDefaultLiveContentTruncateSize;
  }

  if (content.size()) {
    live->set_content_size(content.size());
  }

  if (content.data()) {
    live->set_content_ptr(reinterpret_cast<int64_t>(content.data()));
  }

  if (content.size()) {
    if (Unicode::IsStructurallyValidUtf8(content)) {
      if (content.size() <= truncate_size) {
        content.CopyToString(live->mutable_content());
      } else {
        size_t valid_truncate_size = truncate_size;
        while (valid_truncate_size &&
               ((content[valid_truncate_size] & 0x80) != 0)) {
          --valid_truncate_size;
        }
        content.substr(0, valid_truncate_size)
            .CopyToString(live->mutable_truncated_content());
      }
    } else {
      if (content.size() <= truncate_size) {
        live->set_content_hex(WriteHexBlob(content));
      } else {
        live->set_truncated_content_hex(
            WriteHexBlob(content.substr(0, truncate_size)));
      }
    }
  }
}

void CaptureLiveCallbackOut(const internal::CallbackBase& callback,
                            LiveCallback* live) {
  PJCORE_CHECK(live);
  live->Clear();

  if (GetCallbackBindStateBase(callback)) {
    live->set_bind_state_ptr(
        reinterpret_cast<int64_t>(GetCallbackBindStateBase(callback)));
  }

  if (GetCallbackPolymorphicInvoke(callback)) {
    live->set_polymorphic_invoke_ptr(
        reinterpret_cast<int64_t>(GetCallbackPolymorphicInvoke(callback)));
  }
}

}  // namespace pjcore
