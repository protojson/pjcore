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

#ifndef PJCORE_ERROR_UTIL_H_
#define PJCORE_ERROR_UTIL_H_

#include <string>

#include "pjcore/error.pb.h"

namespace pjcore {

bool ErrnosResolved(const Error& error);

void ResolveErrnos(Error* mutable_error);

std::string ErrorToString(const Error& error,
                          bool strip_quotes_unescape_tabs_and_slashes = true);

void PrintError(const Error& error);

}  // namespace pjcore

#endif  // PJCORE_ERROR_UTIL_H_
