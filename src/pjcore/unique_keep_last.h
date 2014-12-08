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

#ifndef PJCORE_UNIQUE_KEEP_LAST_H_
#define PJCORE_UNIQUE_KEEP_LAST_H_

#include <algorithm>
#include <functional>

namespace pjcore {

template <typename ForwardIterator, typename BinaryPredicate>
ForwardIterator unique_keep_last(ForwardIterator begin, ForwardIterator end,
                                 BinaryPredicate predicate) {
  using std::swap;

  begin = std::adjacent_find(begin, end, predicate);

  if (begin != end) {
    ForwardIterator same = begin;
    ForwardIterator next = same;
    ++next;

    while (next != end) {
      if (!predicate(*same, *next)) {
        swap(*begin++, *same);
      }
    }
    swap(*begin++, *same);
  }

  return begin;
}

template <typename ForwardIterator>
ForwardIterator unique_keep_last(ForwardIterator begin, ForwardIterator end) {
  return unique_keep_last(
      begin, end,
      std::equal_to<
          typename std::iterator_traits<ForwardIterator>::value_type>());
}

}  // namespace pjcore

#endif  // PJCORE_UNIQUE_KEEP_LAST_H_
