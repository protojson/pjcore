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

#ifndef PJCORE_LIVE_CAPTURABLE_H_
#define PJCORE_LIVE_CAPTURABLE_H_

#include <string>

#include "pjcore/third_party/chromium/scoped_ptr.h"

#include "pjcore/json.pb.h"

namespace pjcore {

class LiveCapturableList;

class LiveCapturable {
 public:
  virtual ~LiveCapturable();

  const char* live_class() const { return live_class_; }

  LiveCapturableList* live_list() const { return live_list_; }

  uint64_t live_id() const { return live_id_; }

  uint64_t live_parent_id() const { return live_parent_id_; }

  uint64_t live_parent_ptr() const { return live_parent_ptr_; }

  void set_live_parent(const LiveCapturable* live_parent);

  JsonValue CaptureLiveJson() const;

  void CaptureLiveJsonOut(JsonValue* live_json) const;

  void LogLiveJson(const char* filename, int line) const;

#define PJCORE_LOG_LIVE_JSON         \
  do {                               \
    LogLiveJson(__FILE__, __LINE__); \
  } while (false)

  void PrintLiveJson() const;

 protected:
  LiveCapturable(const char* live_class, LiveCapturableList* live_list);

  void LogDestroy();

  virtual scoped_ptr<google::protobuf::Message> CaptureLive() const = 0;

 private:
  friend class LiveCapturableList;

  LiveCapturable();

  void LogLiveJson(const char* filename, int line, bool on_destroy) const;

  const char* live_class_;

  LiveCapturableList* live_list_;

  LiveCapturable* live_previous_;

  LiveCapturable* live_next_;

  uint64_t live_id_;

  uint64_t live_parent_id_;

  uint64_t live_parent_ptr_;

  bool destroy_logged_;
};

class LiveCapturableList : private LiveCapturable {
 public:
  static const uint64_t kNullId = static_cast<uint64_t>(-1);

  explicit LiveCapturableList(bool log_create_destroy);

  ~LiveCapturableList();

  JsonValue CaptureAllLiveJson() const;

  void CaptureAllLiveJsonOut(JsonValue* live_json) const;

  bool log_create_destroy() const { return log_create_destroy_; }

  void set_log_create_destroy(bool log_create_destroy) {
    log_create_destroy_ = log_create_destroy;
  }

  uint64_t next_id() const { return next_id_; }

  uint64_t size() const { return size_; }

  void LogAllLiveJson(const char* filename, int line) const;

#define PJCORE_LOG_ALL_LIVE_JSON                       \
  do {                                                 \
    if (live_list()) {                                 \
      live_list()->LogAllLiveJson(__FILE__, __LINE__); \
    }                                                  \
  } while (false)

  void PrintAllLiveJson() const;

  static std::string HexifyPtr(uint64_t ptr);

  static std::string HexifyPtr(const void* ptr);

  static void HexifyPtrRecursive(JsonValue* value);

  static void ConsiderHexifyPtrRecursive(JsonValue* value);

 protected:
  scoped_ptr<google::protobuf::Message> CaptureLive() const OVERRIDE;

 private:
  friend class LiveCapturable;

  bool log_create_destroy_;

  uint64_t next_id_;

  uint64_t size_;
};

}  // namespace pjcore

#endif  // PJCORE_LIVE_CAPTURABLE_H_
