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

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <arpa/inet.h>
#endif

#include "pjcore/shared_addr_info_list.h"

#include <string>

#include "pjcore/errno_description.h"
#include "pjcore/number_util.h"

namespace pjcore {

void CaptureLiveAddrInfoOut(const addrinfo* addr_info, LiveAddrInfo* live) {
  PJCORE_CHECK(addr_info);

  PJCORE_CHECK(live);
  live->Clear();

  live->set_ptr(reinterpret_cast<uint64_t>(addr_info));
  if (addr_info->ai_flags) {
    live->set_flags(addr_info->ai_flags);
  }

#define CONSIDER_FIELD(_FIELD_ENUM, _FIELD_NAME)         \
  do {                                                   \
    if ((addr_info->ai_flags & AI_##_FIELD_ENUM) != 0) { \
      live->set_flag_##_FIELD_NAME(true);                \
    }                                                    \
  } while (false)

  CONSIDER_FIELD(PASSIVE, passive);
  CONSIDER_FIELD(CANONNAME, canonname);
  CONSIDER_FIELD(NUMERICHOST, numerichost);
  CONSIDER_FIELD(NUMERICSERV, numericserv);
  CONSIDER_FIELD(ALL, all);
  CONSIDER_FIELD(ADDRCONFIG, addrconfig);
  CONSIDER_FIELD(V4MAPPED, v4mapped);

#undef CONSIDER_FIELD

  if (LiveAddrInfo::FamilyType_IsValid(addr_info->ai_family)) {
    live->set_family(
        static_cast<LiveAddrInfo::FamilyType>(addr_info->ai_family));
  }

  if (LiveAddrInfo::SocketType_IsValid(addr_info->ai_socktype)) {
    live->set_socket(
        static_cast<LiveAddrInfo::SocketType>(addr_info->ai_socktype));
  }

  if (LiveAddrInfo::ProtocolType_IsValid(addr_info->ai_protocol)) {
    live->set_protocol(
        static_cast<LiveAddrInfo::ProtocolType>(addr_info->ai_protocol));
  }

  if (addr_info->ai_addr) {
  }

  if (addr_info->ai_addr) {
    switch (addr_info->ai_family) {
      case AF_INET: {
        sockaddr_in* sock_addr =
            reinterpret_cast<sockaddr_in*>(addr_info->ai_addr);

        char buffer[INET_ADDRSTRLEN];
        const char* str = inet_ntop(addr_info->ai_family, &sock_addr->sin_addr,
                                    buffer, sizeof(buffer));

        if (str) {
          live->set_addr(str);
        } else {
          live->set_addr(
              std::string("Failed to convert IPv4 address to text form: ") +
              CurrentErrnoDescription());
        }

        if (sock_addr->sin_port) {
          live->set_port(ntohs(sock_addr->sin_port));
        }

        break;
      }

      case AF_INET6: {
        sockaddr_in6* sock_addr =
            reinterpret_cast<sockaddr_in6*>(addr_info->ai_addr);

        char buffer[INET6_ADDRSTRLEN];
        const char* str = inet_ntop(addr_info->ai_family, &sock_addr->sin6_addr,
                                    buffer, sizeof(buffer));

        if (str) {
          live->set_addr(str);
        } else {
          live->set_addr(
              std::string("Failed to convert IPv6 address to text form: ") +
              CurrentErrnoDescription());
        }

        if (sock_addr->sin6_port) {
          live->set_port(ntohs(sock_addr->sin6_port));
        }

        if (sock_addr->sin6_flowinfo) {
          live->set_flowinfo(sock_addr->sin6_flowinfo);
        }

        break;
      }

      default: {
        live->set_addr(
            std::string("Cannot convert address family to text form: ") +
            WriteNumber(addr_info->ai_family));
        break;
      }
    }
  }

  if (addr_info->ai_canonname) {
    live->set_canonname(addr_info->ai_canonname);
  }
}

AddrInfoList::AddrInfoList(AbstractUv* uv, addrinfo* list)
    : uv_(uv), list_(list) {}

AddrInfoList::~AddrInfoList() {
  uv()->freeaddrinfo(list_);
  list_ = NULL;
}

void AddrInfoList::CaptureLiveOut(
    google::protobuf::RepeatedPtrField<LiveAddrInfo>* live) const {
  PJCORE_CHECK(live);
  for (const addrinfo* addr_info = list_; addr_info;
       addr_info = addr_info->ai_next) {
    CaptureLiveAddrInfoOut(addr_info, live->Add());
  }
}

}  // namespace pjcore
