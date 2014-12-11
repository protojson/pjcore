// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
//

//
// Deal with the differences between Microsoft and GNU implemenations
// of hash_map. Allows all platforms to use |base::hash_map| and
// |base::hash_set|.
//  eg:
//   base::hash_map<int> my_map;
//   base::hash_set<int> my_set;
//
// NOTE: It is an explicit non-goal of this class to provide a generic hash
// function for pointers.  If you want to hash a pointers to a particular class,
// please define the template specialization elsewhere (for example, in its
// header file) and keep it specific to just pointers to that class.  This is
// because identity hashes are not desirable for all types that might show up
// in containers as pointers.

#ifndef PJCORE_THIRD_PARTY_CHROMIUM_CONTAINERS_HASH_TABLES_H_
#define PJCORE_THIRD_PARTY_CHROMIUM_CONTAINERS_HASH_TABLES_H_

#include <stdint.h>

#include <utility>

#include "pjcore/third_party/chromium/build_config.h"

#if defined(COMPILER_MSVC)
#include <hash_map>
#include <hash_set>

#define PJCORE_HASH_NAMESPACE stdext

#elif defined(COMPILER_GCC)
#if defined(OS_ANDROID)
#define PJCORE_HASH_NAMESPACE std
#else
#define PJCORE_HASH_NAMESPACE __gnu_cxx
#endif

// This is a hack to disable the gcc 4.4 warning about hash_map and hash_set
// being deprecated.  We can get rid of this when we upgrade to VS2008 and we
// can use <tr1/unordered_map> and <tr1/unordered_set>.
#ifdef __DEPRECATED
#define PJCORE_OLD__DEPRECATED __DEPRECATED
#undef __DEPRECATED
#endif

#if defined(OS_ANDROID)
#include <hash_map>
#include <hash_set>
#else
#include <ext/hash_map>
#include <ext/hash_set>
#endif

#include <string>

#ifdef PJCORE_OLD__DEPRECATED
#define __DEPRECATED PJCORE_OLD__DEPRECATED
#undef PJCORE_OLD__DEPRECATED
#endif

#else  // COMPILER
#error define PJCORE_PJCORE_HASH_NAMESPACE for your compiler
#endif  // COMPILER

namespace pjcore {
using PJCORE_HASH_NAMESPACE::hash_map;
using PJCORE_HASH_NAMESPACE::hash_multimap;
using PJCORE_HASH_NAMESPACE::hash_multiset;
using PJCORE_HASH_NAMESPACE::hash_set;

// Implement hashing for pairs of at-most 32 bit integer values.
// When size_t is 32 bits, we turn the 64-bit hash code into 32 bits by using
// multiply-add hashing. This algorithm, as described in
// Theorem 4.3.3 of the thesis "Über die Komplexität der Multiplikation in
// eingeschränkten Branchingprogrammmodellen" by Woelfel, is:
//
//   h32(x32, y32) = (h64(x32, y32) * rand_odd64 + rand16 * 2^16) % 2^64 / 2^32
//
// Contact danakj@chromium.org for any questions.
inline std::size_t HashInts32(uint32_t value1, uint32_t value2) {
  uint64_t value1_64 = value1;
  uint64_t hash64 = (value1_64 << 32) | value2;

  if (sizeof(std::size_t) >= sizeof(uint64_t))
    return static_cast<std::size_t>(hash64);

  uint64_t odd_random = 481046412LL << 32 | 1025306955LL;
  uint32_t shift_random = 10121U << 16;

  hash64 = hash64 * odd_random + shift_random;
  std::size_t high_bits = static_cast<std::size_t>(
      hash64 >> (8 * (sizeof(uint64_t) - sizeof(std::size_t))));
  return high_bits;
}

// Implement hashing for pairs of up-to 64-bit integer values.
// We use the compound integer hash method to produce a 64-bit hash code, by
// breaking the two 64-bit inputs into 4 32-bit values:
// http://opendatastructures.org/versions/edition-0.1d/ods-java/node33.html#SECTION00832000000000000000
// Then we reduce our result to 32 bits if required, similar to above.
inline std::size_t HashInts64(uint64_t value1, uint64_t value2) {
  uint32_t short_random1 = 842304669U;
  uint32_t short_random2 = 619063811U;
  uint32_t short_random3 = 937041849U;
  uint32_t short_random4 = 3309708029U;

  uint32_t value1a = static_cast<uint32_t>(value1 & 0xffffffff);
  uint32_t value1b = static_cast<uint32_t>((value1 >> 32) & 0xffffffff);
  uint32_t value2a = static_cast<uint32_t>(value2 & 0xffffffff);
  uint32_t value2b = static_cast<uint32_t>((value2 >> 32) & 0xffffffff);

  uint64_t product1 = static_cast<uint64_t>(value1a) * short_random1;
  uint64_t product2 = static_cast<uint64_t>(value1b) * short_random2;
  uint64_t product3 = static_cast<uint64_t>(value2a) * short_random3;
  uint64_t product4 = static_cast<uint64_t>(value2b) * short_random4;

  uint64_t hash64 = product1 + product2 + product3 + product4;

  if (sizeof(std::size_t) >= sizeof(uint64_t))
    return static_cast<std::size_t>(hash64);

  uint64_t odd_random = 1578233944LL << 32 | 194370989LL;
  uint32_t shift_random = 20591U << 16;

  hash64 = hash64 * odd_random + shift_random;
  std::size_t high_bits = static_cast<std::size_t>(
      hash64 >> (8 * (sizeof(uint64_t) - sizeof(std::size_t))));
  return high_bits;
}

#define PJCORE_DEFINE_32BIT_PAIR_HASH(Type1, Type2) \
inline std::size_t HashPair(Type1 value1, Type2 value2) { \
  return HashInts32(value1, value2); \
}

PJCORE_DEFINE_32BIT_PAIR_HASH(int16_t, int16_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(int16_t, uint16_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(int16_t, int32_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(int16_t, uint32_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(uint16_t, int16_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(uint16_t, uint16_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(uint16_t, int32_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(uint16_t, uint32_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(int32_t, int16_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(int32_t, uint16_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(int32_t, int32_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(int32_t, uint32_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(uint32_t, int16_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(uint32_t, uint16_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(uint32_t, int32_t);
PJCORE_DEFINE_32BIT_PAIR_HASH(uint32_t, uint32_t);

#undef PJCORE_DEFINE_32BIT_PAIR_HASH

#define PJCORE_DEFINE_64BIT_PAIR_HASH(Type1, Type2) \
inline std::size_t HashPair(Type1 value1, Type2 value2) { \
  return HashInts64(value1, value2); \
}

PJCORE_DEFINE_64BIT_PAIR_HASH(int16_t, int64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int16_t, uint64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint16_t, int64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint16_t, uint64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int32_t, int64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int32_t, uint64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint32_t, int64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint32_t, uint64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int64_t, int16_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int64_t, uint16_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int64_t, int32_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int64_t, uint32_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int64_t, int64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(int64_t, uint64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint64_t, int16_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint64_t, uint16_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint64_t, int32_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint64_t, uint32_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint64_t, int64_t);
PJCORE_DEFINE_64BIT_PAIR_HASH(uint64_t, uint64_t);

#undef PJCORE_DEFINE_64BIT_PAIR_HASH
}  // namespace pjcore

#undef PJCORE_DEFINE_PAIR_HASH_FUNCTION_START
#undef PJCORE_DEFINE_PAIR_HASH_FUNCTION_END

#endif  // PJCORE_THIRD_PARTY_CHROMIUM_CONTAINERS_HASH_TABLES_H_
