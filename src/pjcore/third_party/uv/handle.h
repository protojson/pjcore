// Modification of uv-common.h
// From https://github.com/joyent/libuv/blob/master/src/uv-common.h
// Original contains the following copyright, notice and disclaimer:

/* Copyright Joyent, Inc. and other Node contributors. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

/*
 * This file is private to libuv. It provides common functionality to both
 * Windows and Unix backends.
 */

#ifndef PJCORE_THIRD_PARTY_UV_HANDLE_H_
#define PJCORE_THIRD_PARTY_UV_HANDLE_H_

namespace pjcore {

#ifndef _WIN32
enum {
  UV__HANDLE_INTERNAL = 0x8000,
  UV__HANDLE_ACTIVE = 0x4000,
  UV__HANDLE_REF = 0x2000,
  UV__HANDLE_CLOSING = 0 /* no-op on unix */
};
#else
enum {
  UV__HANDLE_INTERNAL = 0x80,
  UV__HANDLE_ACTIVE = 0x40,
  UV__HANDLE_REF = 0x20,
  UV__HANDLE_CLOSING = 0x01
};
#endif

}  // namespace pjcore

#endif /* PJCORE_THIRD_PARTY_UV_HANDLE_H_ */
