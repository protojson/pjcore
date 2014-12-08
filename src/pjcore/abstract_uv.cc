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

#include "pjcore/abstract_uv.h"

namespace pjcore {

namespace {

class RealUv : public AbstractUv {
 public:
  uv_loop_t* default_loop() OVERRIDE { return ::uv_default_loop(); }
  int loop_init(uv_loop_t* loop) { return ::uv_loop_init(loop); }
  int loop_close(uv_loop_t* loop) { return ::uv_loop_close(loop); }
  uv_loop_t* loop_new() OVERRIDE { return ::uv_loop_new(); }
  void loop_delete(uv_loop_t* loop) { ::uv_loop_delete(loop); }
  int run(uv_loop_t* loop, uv_run_mode mode) { return ::uv_run(loop, mode); }
  void stop(uv_loop_t* loop) { ::uv_stop(loop); }
  void close(uv_handle_t* handle, uv_close_cb close_cb) {
    ::uv_close(handle, close_cb);
  }
  int listen(uv_stream_t* stream, int backlog, uv_connection_cb cb) {
    return ::uv_listen(stream, backlog, cb);
  }
  int accept(uv_stream_t* server, uv_stream_t* client) {
    return ::uv_accept(server, client);
  }
  int read_start(uv_stream_t* stream, uv_alloc_cb alloc_cb,
                 uv_read_cb read_cb) {
    return ::uv_read_start(stream, alloc_cb, read_cb);
  }
  int read_stop(uv_stream_t* stream) { return ::uv_read_stop(stream); }
  int write(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[],
            unsigned int nbufs, uv_write_cb cb) {
    return ::uv_write(req, handle, bufs, nbufs, cb);
  }
  int write2(uv_write_t* req, uv_stream_t* handle, const uv_buf_t bufs[],
             unsigned int nbufs, uv_stream_t* send_handle, uv_write_cb cb) {
    return ::uv_write2(req, handle, bufs, nbufs, send_handle, cb);
  }
  int tcp_init(uv_loop_t* loop, uv_tcp_t* handle) {
    return ::uv_tcp_init(loop, handle);
  }
  int tcp_open(uv_tcp_t* handle, uv_os_sock_t sock) {
    return ::uv_tcp_open(handle, sock);
  }
  int tcp_bind(uv_tcp_t* handle, const struct sockaddr* addr,
               unsigned int flags) {
    return ::uv_tcp_bind(handle, addr, flags);
  }
  int tcp_connect(uv_connect_t* req, uv_tcp_t* handle,
                  const struct sockaddr* addr, uv_connect_cb cb) {
    return ::uv_tcp_connect(req, handle, addr, cb);
  }
  int idle_init(uv_loop_t* loop, uv_idle_t* idle) {
    return ::uv_idle_init(loop, idle);
  }
  int idle_start(uv_idle_t* idle, uv_idle_cb cb) {
    return ::uv_idle_start(idle, cb);
  }
  int idle_stop(uv_idle_t* idle) { return ::uv_idle_stop(idle); }
  int async_init(uv_loop_t* loop, uv_async_t* async, uv_async_cb async_cb) {
    return ::uv_async_init(loop, async, async_cb);
  }
  int async_send(uv_async_t* async) { return ::uv_async_send(async); }
  int timer_init(uv_loop_t* loop, uv_timer_t* handle) {
    return ::uv_timer_init(loop, handle);
  }
  int timer_start(uv_timer_t* handle, uv_timer_cb cb, uint64_t timeout,
                  uint64_t repeat) {
    return ::uv_timer_start(handle, cb, timeout, repeat);
  }
  int timer_stop(uv_timer_t* handle) { return ::uv_timer_stop(handle); }
  int timer_again(uv_timer_t* handle) { return ::uv_timer_again(handle); }
  void timer_set_repeat(uv_timer_t* handle, uint64_t repeat) {
    ::uv_timer_set_repeat(handle, repeat);
  }
  uint64_t timer_get_repeat(const uv_timer_t* handle) {
    return ::uv_timer_get_repeat(handle);
  }
  int getaddrinfo(uv_loop_t* loop, uv_getaddrinfo_t* req,
                  uv_getaddrinfo_cb getaddrinfo_cb, const char* node,
                  const char* service, const struct addrinfo* hints) {
    return ::uv_getaddrinfo(loop, req, getaddrinfo_cb, node, service, hints);
  }
  void freeaddrinfo(struct addrinfo* ai) { ::uv_freeaddrinfo(ai); }
  int getnameinfo(uv_loop_t* loop, uv_getnameinfo_t* req,
                  uv_getnameinfo_cb getnameinfo_cb, const struct sockaddr* addr,
                  int flags) {
    return ::uv_getnameinfo(loop, req, getnameinfo_cb, addr, flags);
  }
  int queue_work(uv_loop_t* loop, uv_work_t* req, uv_work_cb work_cb,
                 uv_after_work_cb after_work_cb) {
    return ::uv_queue_work(loop, req, work_cb, after_work_cb);
  }
  int cancel(uv_req_t* req) { return ::uv_cancel(req); }
  int ip4_addr(const char* ip, int port, struct sockaddr_in* addr) {
    return ::uv_ip4_addr(ip, port, addr);
  }
  int ip6_addr(const char* ip, int port, struct sockaddr_in6* addr) {
    return ::uv_ip6_addr(ip, port, addr);
  }
  int ip4_name(const struct sockaddr_in* src, char* dst, size_t size) {
    return ::uv_ip4_name(src, dst, size);
  }
  int ip6_name(const struct sockaddr_in6* src, char* dst, size_t size) {
    return ::uv_ip6_name(src, dst, size);
  }
};

RealUv g_real_uv;

}  // unnamed namespace

AbstractUv* GetRealUv() { return &g_real_uv; }

}  // namespace pjcore
