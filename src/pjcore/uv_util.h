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

#ifndef PJCORE_UV_UTIL_H_
#define PJCORE_UV_UTIL_H_

#include "pjcore/third_party/chromium/string_piece.h"
#include "pjcore/third_party/chromium/scoped_ptr.h"

#include "pjcore/abstract_uv.h"
#include "pjcore/live_uv.pb.h"

namespace pjcore {

uv_buf_t StringPieceToUvBuf(StringPiece str);

void CaptureLiveUvBufOut(const uv_buf_t& buf, LiveUvBuf* live);

const char* UvHandleTypeStr(uv_handle_type type);
const char* UvReqTypeStr(uv_req_type type);

template <typename Handle>
void DeleteUvHandle(uv_handle_t* handle) {
  delete reinterpret_cast<Handle*>(handle);
}

template <typename Handle>
void CloseAndDeleteUvHandle(AbstractUv* uv, scoped_ptr<Handle> handle) {
  Handle* local_handle = handle.release();
  if (local_handle) {
    uv->close(reinterpret_cast<uv_handle_t*>(local_handle),
              &DeleteUvHandle<Handle>);
  }
}

extern const uv_thread_t kNullUvThread;

void CaptureLiveUvHandleOut(const uv_handle_t* handle, LiveUvHandle* live);
void CaptureLiveUvReqOut(const uv_req_t* req, LiveUvReq* live);

}  // namespace pjcore

#endif  // PJCORE_UV_UTIL_H_
