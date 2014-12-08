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

#include "pjcore/errno_description.h"

#include <errno.h>
#include <string.h>

#include "pjcore/number_util.h"

namespace pjcore {

int CurrentErrno() { return errno; }

std::string ErrnoDescription(int errnum) {
  int original_errno = errno;

  std::string description = std::string("errno ") + WriteNumber(errnum) + ": ";

  char strerrbuf[2048];

#if defined(__APPLE__) || defined(__FREEBSD__) || \
    ((_POSIX_C_SOURCE >= 200112L || _XOPEN_SOURCE >= 600) && !_GNU_SOURCE)

  int result = strerror_r(errnum, strerrbuf, sizeof(strerrbuf));

  if (result) {
    description += strerrbuf;
  } else {
    description += "Unknown error, strerror_r errno ";
    description += WriteNumber(errno);
  }
#elif defined(_WIN32)
  int result = strerror_s(strerrbuf, sizeof(strerrbuf), errnum);

  if (result) {
    description += strerrbuf;
  } else {
    description += "Unknown error, strerror_s errno ";
    description += WriteNumber(errno);
  }
#else
  char* result = strerror_r(errnum, strerrbuf, sizeof(strerrbuf));
  description.append(result);
#endif

  errno = original_errno;

  return description;
}

std::string CurrentErrnoDescription() { return ErrnoDescription(errno); }

}  // namespace pjcore
