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

#include "pjcore_test/mock_uv_base.h"

#include "pjcore/logging.h"

namespace pjcore {

namespace {

MockUvBase g_fatal_uv;

MockUvLoopNewDelete g_mock_uv_loop_new_delete;
}
// unnamed namespace

uv_loop_t* MockUvBase::default_loop() {
  PJCORE_FATALITY("No mock for uv_default_loop");
  return NULL;
}

int MockUvBase::loop_init(uv_loop_t* /* loop */) {
  PJCORE_FATALITY("No mock for uv_loop_init");
  return 0;
}

int MockUvBase::loop_close(uv_loop_t* /* loop */) {
  PJCORE_FATALITY("No mock for uv_loop_close");
  return 0;
}

uv_loop_t* MockUvBase::loop_new() {
  PJCORE_FATALITY("No mock for uv_loop_new");
  return NULL;
}

void MockUvBase::loop_delete(uv_loop_t* /* loop */) {
  PJCORE_FATALITY("No mock for uv_loop_delete");
}

int MockUvBase::run(uv_loop_t* /* loop */, uv_run_mode /* mode */) {
  PJCORE_FATALITY("No mock for uv_run");
  return 0;
}

void MockUvBase::close(uv_handle_t* /* handle */, uv_close_cb /* close_cb */) {
  PJCORE_FATALITY("No mock for uv_close");
}

int MockUvBase::listen(uv_stream_t* /* stream */, int /* backlog */,
                       uv_connection_cb /* cb */) {
  PJCORE_FATALITY("No mock for uv_listen");
  return 0;
}

int MockUvBase::accept(uv_stream_t* /* server */, uv_stream_t* /* client */) {
  PJCORE_FATALITY("No mock for uv_accept");
  return 0;
}

int MockUvBase::read_start(uv_stream_t* /* stream */,
                           uv_alloc_cb /* alloc_cb */,
                           uv_read_cb /* read_cb */) {
  PJCORE_FATALITY("No mock for uv_read_start");
  return 0;
}

int MockUvBase::read_stop(uv_stream_t* /* stream */) {
  PJCORE_FATALITY("No mock for uv_read_stop");
  return 0;
}

int MockUvBase::write(uv_write_t* /* req */, uv_stream_t* /* handle */,
                      const uv_buf_t /* bufs */[], unsigned int /* nbufs */,
                      uv_write_cb /* cb */) {
  PJCORE_FATALITY("No mock for uv_write");
  return 0;
}

int MockUvBase::tcp_init(uv_loop_t* /* loop */, uv_tcp_t* /* handle */) {
  PJCORE_FATALITY("No mock for uv_tcp_init");
  return 0;
}

int MockUvBase::tcp_open(uv_tcp_t* /* handle */, uv_os_sock_t /* sock */) {
  PJCORE_FATALITY("No mock for uv_tcp_open");
  return 0;
}

int MockUvBase::tcp_bind(uv_tcp_t* /* handle */,
                         const struct sockaddr* /* addr */,
                         unsigned int /* flags */) {
  PJCORE_FATALITY("No mock for uv_tcp_bind");
  return 0;
}

int MockUvBase::tcp_connect(uv_connect_t* /* req */, uv_tcp_t* /* handle */,
                            const struct sockaddr* /* addr */,
                            uv_connect_cb /* cb */) {
  PJCORE_FATALITY("No mock for uv_tcp_connect");
  return 0;
}

int MockUvBase::idle_init(uv_loop_t* /* loop */, uv_idle_t* /* idle */) {
  PJCORE_FATALITY("No mock for uv_idle_init");
  return 0;
}

int MockUvBase::idle_start(uv_idle_t* /* idle */, uv_idle_cb /* cb */) {
  PJCORE_FATALITY("No mock for uv_idle_start");
  return 0;
}

int MockUvBase::idle_stop(uv_idle_t* /* idle */) {
  PJCORE_FATALITY("No mock for uv_idle_stop");
  return 0;
}

int MockUvBase::async_init(uv_loop_t* /* loop */, uv_async_t* /* async */,
                           uv_async_cb /* async_cb */) {
  PJCORE_FATALITY("No mock for uv_async_init");
  return 0;
}

int MockUvBase::async_send(uv_async_t* /* async */) {
  PJCORE_FATALITY("No mock for uv_async_send");
  return 0;
}

int MockUvBase::timer_init(uv_loop_t* /* loop */, uv_timer_t* /* handle */) {
  PJCORE_FATALITY("No mock for uv_timer_init");
  return 0;
}

int MockUvBase::timer_start(uv_timer_t* /* handle */, uv_timer_cb /* cb */,
                            uint64_t /* timeout */, uint64_t /* repeat */) {
  PJCORE_FATALITY("No mock for uv_timer_start");
  return 0;
}

int MockUvBase::timer_stop(uv_timer_t* /* handle */) {
  PJCORE_FATALITY("No mock for uv_timer_stop");
  return 0;
}

int MockUvBase::timer_again(uv_timer_t* /* handle */) {
  PJCORE_FATALITY("No mock for uv_timer_again");
  return 0;
}

void MockUvBase::timer_set_repeat(uv_timer_t* /* handle */,
                                  uint64_t /* repeat */) {
  PJCORE_FATALITY("No mock for uv_timer_set_repeat");
}

uint64_t MockUvBase::timer_get_repeat(const uv_timer_t* /* handle */) {
  PJCORE_FATALITY("No mock for uv_timer_get_repeat");
  return 0;
}

int MockUvBase::getaddrinfo(uv_loop_t* /* loop */, uv_getaddrinfo_t* /* req */,
                            uv_getaddrinfo_cb /* getaddrinfo_cb */,
                            const char* /* node */, const char* /* service */,
                            const struct addrinfo* /* hints */) {
  PJCORE_FATALITY("No mock for uv_getaddrinfo");
  return 0;
}

void MockUvBase::freeaddrinfo(struct addrinfo* /* ai */) {
  PJCORE_FATALITY("No mock for uv_freeaddrinfo");
}

int MockUvBase::queue_work(uv_loop_t* /* loop */, uv_work_t* /* req */,
                           uv_work_cb /* work_cb */,
                           uv_after_work_cb /* after_work_cb */) {
  PJCORE_FATALITY("No mock for uv_queue_work");
  return 0;
}

int MockUvBase::cancel(uv_req_t* /* req */) {
  PJCORE_FATALITY("No mock for uv_cancel");
  return 0;
}

int MockUvBase::ip4_addr(const char* /* ip */, int /* port */,
                         struct sockaddr_in* /* addr */) {
  PJCORE_FATALITY("No mock for uv_ip4_addr");
  return 0;
}

int MockUvBase::ip6_addr(const char* /* ip */, int /* port */,
                         struct sockaddr_in6* /* addr */) {
  PJCORE_FATALITY("No mock for uv_ip6_addr");
  return 0;
}

int MockUvBase::ip4_name(const struct sockaddr_in* /* src */, char* /* dst */,
                         size_t /* size */) {
  PJCORE_FATALITY("No mock for uv_ip4_name");
  return 0;
}

int MockUvBase::ip6_name(const struct sockaddr_in6* /* src */, char* /* dst */,
                         size_t /* size */) {
  PJCORE_FATALITY("No mock for uv_ip6_name");
  return 0;
}

uv_loop_t* MockUvLoopNewDelete::loop_new() {
  uv_loop_t* loop = new uv_loop_t();
  memset(loop, 0, sizeof(*loop));
  return loop;
}

void MockUvLoopNewDelete::loop_delete(uv_loop_t* loop) {
  delete loop;
  loop = NULL;
}

AbstractUv* GetFatalUv() { return &g_fatal_uv; }

AbstractUv* GetMockUvLoopNewDelete() { return &g_mock_uv_loop_new_delete; }

}  // namespace pjcore
