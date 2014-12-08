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

#ifndef PJCORE_ABSTRACT_UV_H_
#define PJCORE_ABSTRACT_UV_H_

#include <uv.h>

#include "pjcore/third_party/chromium/compiler_specific.h"

namespace pjcore {

class AbstractUv {
 public:
  virtual ~AbstractUv() {}
  virtual uv_loop_t* default_loop() = 0;
  virtual int loop_init(uv_loop_t* loop) = 0;
  virtual int loop_close(uv_loop_t* loop) = 0;
  virtual uv_loop_t* loop_new() = 0;
  virtual void loop_delete(uv_loop_t* loop) = 0;
  virtual int run(uv_loop_t* loop, uv_run_mode mode) = 0;
  virtual void close(uv_handle_t* handle, uv_close_cb close_cb) = 0;
  virtual int listen(uv_stream_t* stream, int backlog, uv_connection_cb cb) = 0;
  virtual int accept(uv_stream_t* server, uv_stream_t* client) = 0;
  virtual int read_start(uv_stream_t* stream, uv_alloc_cb alloc_cb,
                         uv_read_cb read_cb) = 0;
  virtual int read_stop(uv_stream_t* stream) = 0;
  virtual int write(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[],
                    unsigned int nbufs, uv_write_cb cb) = 0;
  virtual int tcp_init(uv_loop_t* loop, uv_tcp_t* handle) = 0;
  virtual int tcp_open(uv_tcp_t* handle, uv_os_sock_t sock) = 0;
  virtual int tcp_bind(uv_tcp_t* handle, const struct sockaddr* addr,
                       unsigned int flags) = 0;
  virtual int tcp_connect(uv_connect_t* req, uv_tcp_t* handle,
                          const struct sockaddr* addr, uv_connect_cb cb) = 0;
  virtual int idle_init(uv_loop_t* loop, uv_idle_t* idle) = 0;
  virtual int idle_start(uv_idle_t* idle, uv_idle_cb cb) = 0;
  virtual int idle_stop(uv_idle_t* idle) = 0;
  virtual int async_init(uv_loop_t* loop, uv_async_t* async,
                         uv_async_cb async_cb) = 0;
  virtual int async_send(uv_async_t* async) = 0;
  virtual int timer_init(uv_loop_t* loop, uv_timer_t* handle) = 0;
  virtual int timer_start(uv_timer_t* handle, uv_timer_cb cb, uint64_t timeout,
                          uint64_t repeat) = 0;
  virtual int timer_stop(uv_timer_t* handle) = 0;
  virtual int timer_again(uv_timer_t* handle) = 0;
  virtual void timer_set_repeat(uv_timer_t* handle, uint64_t repeat) = 0;
  virtual uint64_t timer_get_repeat(const uv_timer_t* handle) = 0;
  virtual int getaddrinfo(uv_loop_t* loop, uv_getaddrinfo_t* req,
                          uv_getaddrinfo_cb getaddrinfo_cb, const char* node,
                          const char* service,
                          const struct addrinfo* hints) = 0;
  virtual void freeaddrinfo(struct addrinfo* ai) = 0;
  virtual int queue_work(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb,
                         uv_after_work_cb after_work_cb) = 0;
  virtual int cancel(uv_req_t* req) = 0;
  virtual int ip4_addr(const char* ip, int port, struct sockaddr_in* addr) = 0;
  virtual int ip6_addr(const char* ip, int port, struct sockaddr_in6* addr) = 0;
  virtual int ip4_name(const struct sockaddr_in* src, char* dst,
                       size_t size) = 0;
  virtual int ip6_name(const struct sockaddr_in6* src, char* dst,
                       size_t size) = 0;
};

AbstractUv* GetRealUv();

}  // namespace pjcore

#endif  // PJCORE_ABSTRACT_UV_H_
