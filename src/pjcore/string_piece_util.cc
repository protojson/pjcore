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

#include "pjcore/string_piece_util.h"

#include <string>

#include "pjcore/logging.h"
#include "pjcore/error.pb.h"

namespace pjcore {

StringPiece MemMove(void* target, StringPiece source) {
  return StringPiece(
      static_cast<char*>(memmove(target, source.data(), source.size())),
      source.size());
}

bool WriteHexBlob(StringPiece binary, void* hex_blob, size_t hex_blob_size) {
  if (hex_blob_size != binary.size() * 2) {
    PJCORE_FAIL_SILENT("Mismatched hex blob size");
  }

  const uint8_t* p = reinterpret_cast<const uint8_t*>(binary.data());
  const uint8_t* end = p + binary.size();

  char* q = reinterpret_cast<char*>(hex_blob);

  while (p != end) {
    q[0] = WriteHexDigit(p[0] >> 4);
    q[1] = WriteHexDigit(p[0] & 0xf);

    p += 1;
    q += 2;
  }

  return true;
}

std::string WriteHexBlob(StringPiece binary) {
  std::string hex_blob;
  hex_blob.resize(binary.size() * 2);

  PJCORE_CHECK(WriteHexBlob(binary, &(hex_blob[0]), hex_blob.size()));
  return hex_blob;
}

bool ReadHexBlob(StringPiece hex_blob, void* binary, size_t binary_size,
                 Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_REQUIRE(hex_blob.size() % 2 == 0, "Odd hex blob size");

  PJCORE_REQUIRE(binary_size == hex_blob.size() / 2, "Mismatched binary size");

  const char* p = hex_blob.data();
  const char* end = p + hex_blob.size();

  uint8_t* q = reinterpret_cast<uint8_t*>(binary);

  while (p != end) {
    PJCORE_REQUIRE(IsHexDigit::eval(p[0]) && IsHexDigit::eval(p[1]),
                   "Invalid hex digit");

    q[0] = ((ReadHexDigit(p[0]) << 4) | ReadHexDigit(p[1]));

    p += 2;
    q += 1;
  }

  return true;
}

bool ReadHexBlob(StringPiece hex_blob, std::string* binary, Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(binary);

  if (hex_blob.size() % 2 != 0) {
    binary->resize(0);
    PJCORE_FAIL("Odd hex blob size");
  }

  binary->resize(hex_blob.size() / 2);

  if (!ReadHexBlob(hex_blob, &((*binary)[0]), binary->size(), error)) {
    binary->resize(0);
    PJCORE_FAIL_SILENT("Failed to read hex blob");
  }

  return true;
}

std::string WriteBase64(StringPiece binary) {
  std::string base_64((binary.length() + 2) / 3 * 4, '\0');

  const uint8_t* bytes = reinterpret_cast<const uint8_t*>(binary.data());

  size_t segment;
  for (segment = 0; segment < binary.length() / 3; ++segment) {
    uint32_t value = (static_cast<uint32_t>(bytes[segment * 3]) << 16) |
                     (static_cast<uint32_t>(bytes[segment * 3 + 1]) << 8) |
                     static_cast<uint32_t>(bytes[segment * 3 + 2]);
    base_64[segment * 4] = WriteBase64Digit(value >> 18);
    base_64[segment * 4 + 1] = WriteBase64Digit((value >> 12) & 0x3f);
    base_64[segment * 4 + 2] = WriteBase64Digit((value >> 6) & 0x3f);
    base_64[segment * 4 + 3] = WriteBase64Digit(value & 0x3f);
  }

  switch (binary.length() % 3) {
    case 0:
      break;

    case 1: {
      uint32_t value = (static_cast<uint32_t>(bytes[segment * 3]) << 16);
      base_64[segment * 4] = WriteBase64Digit(value >> 18);
      base_64[segment * 4 + 1] = WriteBase64Digit((value >> 12) & 0x3f);
      base_64[segment * 4 + 2] = '=';
      base_64[segment * 4 + 3] = '=';
      break;
    }

    case 2: {
      uint32_t value = (static_cast<uint32_t>(bytes[segment * 3]) << 16) |
                       (static_cast<uint32_t>(bytes[segment * 3 + 1]) << 8);
      base_64[segment * 4] = WriteBase64Digit(value >> 18);
      base_64[segment * 4 + 1] = WriteBase64Digit((value >> 12) & 0x3f);
      base_64[segment * 4 + 2] = WriteBase64Digit((value >> 6) & 0x3f);
      base_64[segment * 4 + 3] = '=';
      break;
    } break;
  }

  return base_64;
}

bool ReadBase64(StringPiece base_64, std::string* binary, Error* error) {
  PJCORE_CHECK(binary);

  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_REQUIRE(base_64.length() % 4 == 0, "Base64 length not divisible by 4");

  size_t padding_length = 0;
  if (!base_64.empty() && base_64[base_64.length() - 1] == '=') {
    padding_length = (base_64[base_64.length() - 2] == '=') ? 2 : 1;
  }

  binary->assign((base_64.length() + 3) / 4 * 3 - padding_length, '\0');

  size_t segment;
  for (segment = 0; segment < binary->length() / 3; ++segment) {
    PJCORE_REQUIRE(IsBase64Digit::eval(base_64[segment * 4]) &&
                       IsBase64Digit::eval(base_64[segment * 4 + 1]) &&
                       IsBase64Digit::eval(base_64[segment * 4 + 2]) &&
                       IsBase64Digit::eval(base_64[segment * 4 + 3]),
                   "Invalid Base64 digit");

    uint32_t value =
        (static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4])) << 18) |
        (static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4 + 1]))
         << 12) |
        (static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4 + 2]))
         << 6) |
        static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4 + 3]));

    (*binary)[segment * 3] = static_cast<char>(value >> 16);
    (*binary)[segment * 3 + 1] = static_cast<char>((value >> 8) & 0xff);
    (*binary)[segment * 3 + 2] = static_cast<char>(value & 0xff);
  }

  switch (padding_length) {
    case 0:
      break;

    case 1: {
      PJCORE_REQUIRE(IsBase64Digit::eval(base_64[segment * 4]) &&
                         IsBase64Digit::eval(base_64[segment * 4 + 1]) &&
                         IsBase64Digit::eval(base_64[segment * 4 + 2]),
                     "Invalid Base64 digit");

      uint32_t value =
          (static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4])) << 18) |
          (static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4 + 1]))
           << 12) |
          (static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4 + 2]))
           << 6);

      (*binary)[segment * 3] = static_cast<char>(value >> 16);
      (*binary)[segment * 3 + 1] = static_cast<char>((value >> 8) & 0xff);
      PJCORE_REQUIRE((value & 0xff) == 0, "Non-zero Base64 trailing sequence");
      break;
    }

    case 2: {
      PJCORE_REQUIRE(IsBase64Digit::eval(base_64[segment * 4]) &&
                         IsBase64Digit::eval(base_64[segment * 4 + 1]),
                     "Invalid Base64 digit");

      uint32_t value =
          (static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4])) << 18) |
          (static_cast<uint32_t>(ReadBase64Digit(base_64[segment * 4 + 1]))
           << 12);

      (*binary)[segment * 3] = static_cast<char>(value >> 16);
      PJCORE_REQUIRE(((value >> 8) & 0xff) == 0,
                     "Non-zero Base64 trailing sequence");
      PJCORE_REQUIRE((value & 0xff) == 0, "Non-zero Base64 trailing sequence");
      break;
    }
  }

  return true;
}

}  // namespace pjcore
