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

#ifndef PJCORE_NAME_VALUE_UTIL_H_
#define PJCORE_NAME_VALUE_UTIL_H_

#include <algorithm>
#include <functional>
#include <string>
#include <utility>

#include "google/protobuf/repeated_field.h"

#include "pjcore/third_party/chromium/string_piece.h"

#include "pjcore/logging.h"

namespace pjcore {

template <typename WithName, typename Name>
const WithName* FindByName(
    const google::protobuf::RepeatedPtrField<WithName>& list,
    const Name& name) {
  for (
      typename google::protobuf::RepeatedPtrField<WithName>::const_iterator it =
          list.begin();
      it != list.end(); ++it) {
    if (it->name() == name) {
      return &*it;
    }
  }
  return NULL;
}

template <typename WithName, typename Name>
WithName* FindByName(google::protobuf::RepeatedPtrField<WithName>* list,
                     const Name& name) {
  PJCORE_CHECK(list);
  for (typename google::protobuf::RepeatedPtrField<WithName>::iterator it =
           list->begin();
       it != list->end(); ++it) {
    if (it->name() == name) {
      return &*it;
    }
  }
  return NULL;
}

template <typename Pair, typename Name>
Pair* PushBackName(google::protobuf::RepeatedPtrField<Pair>* list,
                   const Name& name) {
  PJCORE_CHECK(list);
  Pair* pair = list->Add();

  pair->set_name(name);

  return pair;
}

template <typename WithName>
struct LessByName {
  bool operator()(const WithName& left, const WithName& right) const {
    return left.name() < right.name();
  }
};

template <typename WithName>
struct EqualToByName {
  bool operator()(const WithName& left, const WithName& right) const {
    return left.name() == right.name();
  }
};

template <typename WithName>
void SortByName(google::protobuf::RepeatedPtrField<WithName>* list) {
  std::sort(list->begin(), list->end(), LessByName<WithName>());
}

template <typename WithName>
void StableSortByName(google::protobuf::RepeatedPtrField<WithName>* list) {
  std::stable_sort(list->begin(), list->end(), LessByName<WithName>());
}

template <typename WithName>
void RemoveDuplicatesByName(
    google::protobuf::RepeatedPtrField<WithName>* list) {
  int new_size = static_cast<int>(
      std::unique(list->begin(), list->end(), EqualToByName<WithName>()) -
      list->begin());

  while (list->size() > new_size) {
    list->RemoveLast();
  }
}

template <typename WithName>
void SortAndRemoveDuplicatesByName(
    google::protobuf::RepeatedPtrField<WithName>* list) {
  SortByName<WithName>(list);
  RemoveDuplicatesByName<WithName>(list);
}

template <typename WithName>
void StableSortAndRemoveDuplicatesByName(
    google::protobuf::RepeatedPtrField<WithName>* list) {
  StableSortByName(list);
  RemoveDuplicatesByName<WithName>(list);
}

template <typename WithName>
bool IsSortedByName(const google::protobuf::RepeatedPtrField<WithName>& list) {
  typename google::protobuf::RepeatedPtrField<WithName>::const_iterator it =
      list.begin();
  if (it == list.end()) {
    return true;
  }
  typename google::protobuf::RepeatedPtrField<WithName>::const_iterator next =
      it;
  ++next;
  for (;;) {
    if (next == list.end()) {
      return true;
    }
    if (LessByName<WithName>()(*next, *it)) {
      return false;
    }
    it = next;
    ++next;
  }
}

template <typename WithName>
bool IsSortedAndNoDuplicatesByName(
    const google::protobuf::RepeatedPtrField<WithName>& list) {
  typename google::protobuf::RepeatedPtrField<WithName>::const_iterator it =
      list.begin();
  if (it == list.end()) {
    return true;
  }
  typename google::protobuf::RepeatedPtrField<WithName>::const_iterator next =
      it;
  ++next;
  for (;;) {
    if (next == list.end()) {
      return true;
    }
    if (!LessByName<WithName>()(*it, *next)) {
      return false;
    }
    it = next;
    ++next;
  }
}

template <typename WithName, typename Name>
std::pair<WithName*, bool> InsertName(
    google::protobuf::RepeatedPtrField<WithName>* list, const Name& name) {
  PJCORE_CHECK(list);
  bool inserted = false;
  int match = 0;
  while (match < list->size() && !(list->Get(match).name() == name)) {
    ++match;
  }
  if (match >= list->size()) {
    list->Add()->set_name(name);
    inserted = true;
  }
  int from = match + 1;
  int to = from;
  while (from < list->size()) {
    if (!(list->Get(from).name() == name)) {
      list->SwapElements(from, to);
      ++to;
    }
    ++from;
  }
  while (to < list->size()) {
    list->RemoveLast();
  }
  return std::pair<WithName*, bool>(list->Mutable(match), inserted);
}

}  // namespace pjcore

#endif  // PJCORE_NAME_VALUE_UTIL_H_
