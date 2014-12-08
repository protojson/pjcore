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

#ifndef PJCORE_SHARED_ADDR_INFO_LIST_H_
#define PJCORE_SHARED_ADDR_INFO_LIST_H_

#include "pjcore/third_party/chromium/ref_counted.h"

#include "pjcore/abstract_uv.h"
#include "pjcore/live_addr_info.pb.h"

namespace pjcore {

void CaptureLiveAddrInfoOut(const addrinfo* addr_info, LiveAddrInfo* live);

class AddrInfoList : public RefCounted<AddrInfoList> {
 public:
  AddrInfoList(AbstractUv* uv, addrinfo* list);

  AbstractUv* uv() const { return uv_; }

  const addrinfo* list() const { return list_; }

  void CaptureLiveOut(
      google::protobuf::RepeatedPtrField<LiveAddrInfo>* list) const;

 private:
  friend class RefCounted<AddrInfoList>;

  ~AddrInfoList();

  AbstractUv* uv_;

  addrinfo* list_;
};

typedef scoped_refptr<AddrInfoList> SharedAddrInfoList;

}  // namespace pjcore

#endif  // PJCORE_SHARED_ADDR_INFO_LIST_H_
