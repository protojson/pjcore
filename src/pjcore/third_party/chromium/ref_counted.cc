// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cassert>

#include "pjcore/third_party/chromium/ref_counted.h"

namespace pjcore {

namespace subtle {

bool RefCountedThreadSafeBase::HasOneRef() const {
  return base::AtomicRefCountIsOne(
      &const_cast<RefCountedThreadSafeBase*>(this)->ref_count_);
}

RefCountedThreadSafeBase::RefCountedThreadSafeBase() : ref_count_(0) {
#ifndef NDEBUG
  in_dtor_ = false;
#endif
}

RefCountedThreadSafeBase::~RefCountedThreadSafeBase() {
#ifndef NDEBUG
  assert(in_dtor_);  // RefCountedThreadSafe object deleted without calling
                     // Release()
#endif
}

void RefCountedThreadSafeBase::AddRef() const {
#ifndef NDEBUG
  assert(!in_dtor_);
#endif
  base::AtomicRefCountInc(&ref_count_);
}

bool RefCountedThreadSafeBase::Release() const {
#ifndef NDEBUG
  assert(!in_dtor_);
  assert(!base::AtomicRefCountIsZero(&ref_count_));
#endif
  if (!base::AtomicRefCountDec(&ref_count_)) {
#ifndef NDEBUG
    in_dtor_ = true;
#endif
    return true;
  }
  return false;
}

}  // namespace subtle

}  // namespace pjcore
