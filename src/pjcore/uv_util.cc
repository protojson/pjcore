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

#include "pjcore/uv_util.h"

#include "pjcore/third_party/uv/handle.h"

#include "pjcore/logging.h"

namespace pjcore {

uv_buf_t StringPieceToUvBuf(StringPiece str) {
  uv_buf_t buf;
  buf.base = const_cast<char*>(str.data());
  buf.len = str.size();
  return buf;
}

void CaptureLiveUvBufOut(const uv_buf_t& buf, LiveUvBuf* live) {
  PJCORE_CHECK(live);

  live->Clear();
  live->set_base_ptr(reinterpret_cast<int64_t>(buf.base));
  live->set_len(buf.len);
}

const char* UvHandleTypeStr(uv_handle_type type) {
  switch (type) {
#define XX(upper_case, lower_case) \
  case UV_##upper_case:            \
    return #lower_case;            \
    break;
    UV_HANDLE_TYPE_MAP(XX)
#undef XX

    default:
      return "<unknown>";
  }
}

const char* UvReqTypeStr(uv_req_type type) {
  switch (type) {
#define XX(upper_case, lower_case) \
  case UV_##upper_case:            \
    return #lower_case;            \
    break;
    UV_REQ_TYPE_MAP(XX)
#undef XX

    default:
      return "<unknown>";
  }
}

const uv_thread_t kNullUvThread = 0;

void CaptureLiveUvHandleOut(const uv_handle_t* handle, LiveUvHandle* live) {
  PJCORE_CHECK(handle);

  PJCORE_CHECK(live);
  live->Clear();

  live->set_ptr(reinterpret_cast<int64_t>(handle));
  live->set_type(handle->type);
  live->set_type_str(UvHandleTypeStr(handle->type));
  if (handle->data) {
    live->set_data_ptr(reinterpret_cast<int64_t>(handle->data));
  }

  if (handle->close_cb) {
    live->set_close_cb_ptr(reinterpret_cast<int64_t>(handle->close_cb));
  }
  if (handle->loop) {
    live->set_loop_ptr(reinterpret_cast<int64_t>(handle->loop));
  }
  if (handle->flags) {
    int flags = static_cast<int>(handle->flags);
    live->set_flags(handle->flags);
    if (flags & static_cast<int>(UV__HANDLE_INTERNAL)) {
      live->set_flag_internal(true);
    }
    if (flags & static_cast<int>(UV__HANDLE_ACTIVE)) {
      live->set_flag_active(true);
    }
    if (flags & static_cast<int>(UV__HANDLE_REF)) {
      live->set_flag_ref(true);
    }
    if (flags & static_cast<int>(UV__HANDLE_CLOSING)) {
      live->set_flag_closing(true);
    }
  }
}

void CaptureLiveUvReqOut(const uv_req_t* req, LiveUvReq* live) {
  PJCORE_CHECK(req);

  PJCORE_CHECK(live);
  live->Clear();

  live->set_ptr(reinterpret_cast<int64_t>(req));
  live->set_type(req->type);
  live->set_type_str(UvReqTypeStr(req->type));
  if (req->data) {
    live->set_data_ptr(reinterpret_cast<int64_t>(req->data));
  }
}

}  // namespace pjcore
