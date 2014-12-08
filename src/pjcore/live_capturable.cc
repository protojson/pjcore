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

#include "pjcore/live_capturable.h"

#include <inttypes.h>
#include <stdint.h>

#include <algorithm>
#include <string>
#include <typeinfo>

#include "pjcore/logging.h"
#include "pjcore/json_properties.h"
#include "pjcore/json_util.h"
#include "pjcore/json_writer.h"
#include "pjcore/make_json_value.h"
#include "pjcore/number_util.h"

namespace pjcore {

LiveCapturable::~LiveCapturable() {
  if (!destroy_logged_) {
    destroy_logged_ = true;
    if (live_list_ && live_list_->log_create_destroy()) {
      PJCORE_LOG(std::string("LiveCapturable: destroying #") +
                 WriteNumber(live_id_) + " " + live_class_ + " @" +
                 LiveCapturableList::HexifyPtr(this));
    }
  }

  if (live_list_) {
    live_previous_->live_next_ = live_next_;
    live_next_->live_previous_ = live_previous_;

    live_next_ = NULL;
    live_previous_ = NULL;

    --live_list_->size_;
  }
}

void LiveCapturable::set_live_parent(const LiveCapturable* live_parent) {
  if (!live_parent) {
    live_parent_id_ = 0;
    live_parent_ptr_ = 0;
  } else {
    live_parent_id_ = live_parent->live_id();
    live_parent_ptr_ = reinterpret_cast<uint64_t>(live_parent);
  }
}

JsonValue LiveCapturable::CaptureLiveJson() const {
  JsonValue live_json;
  CaptureLiveJsonOut(&live_json);
  return live_json;
}

void LiveCapturable::CaptureLiveJsonOut(JsonValue* live_json) const {
  PJCORE_CHECK(live_json);
  live_json->Clear();

  scoped_ptr<google::protobuf::Message> live(CaptureLive());

  if (live) {
    *live_json = MakeJsonValue(*live);
    PJCORE_CHECK_EQ(JsonValue::TYPE_OBJECT, live_json->type());
  } else {
    *live_json = MakeJsonObject();
  }

  LiveCapturableList::ConsiderHexifyPtrRecursive(live_json);

  JsonPropertyList live_properties;

  if (live_id_ != LiveCapturableList::kNullId) {
    SetJsonProperty(&live_properties, "live_id", live_id_);
  }

  SetJsonProperty(&live_properties, "live_class", live_class_);

  SetJsonProperty(&live_properties, "live_ptr",
                  LiveCapturableList::HexifyPtr(this));

  if (live_parent_id_) {
    SetJsonProperty(&live_properties, "live_parent_id", live_parent_id_);
  }

  if (live_parent_ptr_) {
    SetJsonProperty(&live_properties, "live_parent_ptr",
                    LiveCapturableList::HexifyPtr(live_parent_ptr_));
  }

  for (int index = 0; index < live_properties.size(); ++index) {
    live_json->mutable_object_properties()->Add();
  }

  for (int index = live_json->object_properties_size();
       index > live_properties.size(); --index) {
    live_json->mutable_object_properties()->SwapElements(
        index - live_properties.size() - 1, index - 1);
  }

  for (int index = 0; index < live_properties.size(); ++index) {
    using std::swap;
    swap(*live_json->mutable_object_properties(index),
         *live_properties.Mutable(index));
  }
}

void LiveCapturable::LogLiveJson(const char* filename, int line) const {
  LogLiveJson(filename, line, false);
}

void LiveCapturable::PrintLiveJson() const {
  std::string json_str = WritePrettyJson(CaptureLiveJson());
  fprintf(stdout, "%.*s\n", static_cast<int>(json_str.size()), json_str.data());
}

LiveCapturable::LiveCapturable(const char* live_class,
                               LiveCapturableList* live_list)
    : live_class_(live_class),
      live_list_(live_list),
      live_previous_(NULL),
      live_next_(NULL),
      live_id_(static_cast<size_t>(-1)),
      live_parent_id_(0),
      live_parent_ptr_(0),
      destroy_logged_(false) {
  PJCORE_CHECK(live_class_);

  if (live_list_) {
    live_id_ = live_list_->next_id_++;

    if (live_list_->log_create_destroy()) {
      PJCORE_LOG(std::string("LiveCapturable: constructing #") +
                 WriteNumber(live_id_) + " " + live_class_ + " @" +
                 LiveCapturableList::HexifyPtr(this));
    }

    live_next_ = live_list_;
    live_previous_ = live_list_->live_previous_;

    live_previous_->live_next_ = this;
    live_next_->live_previous_ = this;

    ++live_list_->size_;
  }
}

void LiveCapturable::LogDestroy() {
  if (!destroy_logged_) {
    destroy_logged_ = true;
    if (live_list_ && live_list_->log_create_destroy()) {
      LogLiveJson(__FILE__, __LINE__, true);
    }
  }
}

LiveCapturable::LiveCapturable()
    : live_class_(NULL),
      live_list_(NULL),
      live_previous_(NULL),
      live_next_(NULL),
      live_id_(LiveCapturableList::kNullId) {}

void LiveCapturable::LogLiveJson(const char* filename, int line,
                                 bool on_destroy) const {
  const LogCallback& log_callback = GetGlobalLogCallback();
  if (!log_callback.is_null()) {
    log_callback.Run(
        filename, line,
        std::string("LiveCapturable: ") +
            (on_destroy ? "destroying" : "logging") +
            (live_id_ == LiveCapturableList::kNullId
                 ? std::string()
                 : (std::string(" #") + WriteNumber(live_id_))) +
            " " + live_class_ + " @" + LiveCapturableList::HexifyPtr(this) +
            (on_destroy ? "" : "\n") +
            (on_destroy ? std::string() : WritePrettyJson(CaptureLiveJson())),
        NULL);
  }
}

const uint64_t LiveCapturableList::kNullId;

LiveCapturableList::LiveCapturableList(bool log_create_destroy)
    : LiveCapturable(),
      log_create_destroy_(log_create_destroy),
      next_id_(0),
      size_(0) {
  if (log_create_destroy_) {
    PJCORE_LOG(std::string("LiveCapturable: constructing list @") +
               HexifyPtr(this));
  }

  live_previous_ = this;
  live_next_ = this;
}

LiveCapturableList::~LiveCapturableList() {
  if (log_create_destroy_) {
    PJCORE_LOG(std::string("LiveCapturable: destroying list @") +
               HexifyPtr(this));
  }

  PJCORE_CHECK(this == live_previous_);  // live_previous_->live_id_,
                                         // live_previous_->live_class_,
                                         // HexifyPtr(live_previous_)

  PJCORE_CHECK(this == live_next_);  // HexifyPtr(live_previous_)
}

JsonValue LiveCapturableList::CaptureAllLiveJson() const {
  JsonValue live_json;
  CaptureAllLiveJsonOut(&live_json);
  return live_json;
}

void LiveCapturableList::CaptureAllLiveJsonOut(JsonValue* live_json) const {
  PJCORE_CHECK(live_json);
  live_json->Clear();

  live_json->set_type(JsonValue::TYPE_ARRAY);

  for (const LiveCapturable* capturable = live_next_; capturable != this;
       capturable = capturable->live_next_) {
    PJCORE_CHECK_EQ(this, capturable->live_list_);
    PJCORE_CHECK(capturable->live_previous_);
    PJCORE_CHECK(capturable->live_next_);
    capturable->CaptureLiveJsonOut(live_json->add_array_elements());
  }
}

scoped_ptr<google::protobuf::Message> LiveCapturableList::CaptureLive() const {
  // Sentinel doesn't capture live message.
  PJCORE_CHECK(false);
  return scoped_ptr<google::protobuf::Message>();
}

void LiveCapturableList::LogAllLiveJson(const char* filename, int line) const {
  const LogCallback& log_callback = GetGlobalLogCallback();
  if (!log_callback.is_null()) {
    log_callback.Run(filename, line,
                     std::string("LiveCapturable: logging list @") +
                         HexifyPtr(this) + "\n" +
                         WritePrettyJson(CaptureAllLiveJson()),
                     NULL);
  }
}

void LiveCapturableList::PrintAllLiveJson() const {
  std::string json_str = WritePrettyJson(CaptureAllLiveJson());
  fprintf(stdout, "%.*s\n", static_cast<int>(json_str.size()), json_str.data());
}

std::string LiveCapturableList::HexifyPtr(uint64_t ptr) {
  char buffer[32] = {};
#ifdef _WIN32
  _snprintf(buffer, sizeof(buffer), "0x%016llx", ptr);
#else
  snprintf(buffer, sizeof(buffer), "0x%016llx", ptr);
#endif
  return buffer;
}

std::string LiveCapturableList::HexifyPtr(const void* ptr) {
  return HexifyPtr(reinterpret_cast<uint64_t>(ptr));
}

void LiveCapturableList::HexifyPtrRecursive(JsonValue* value) {
  PJCORE_CHECK(value);

  if (value->type() == JsonValue::TYPE_ARRAY) {
    for (google::protobuf::RepeatedPtrField<JsonValue>::iterator it =
             value->mutable_array_elements()->begin();
         it != value->mutable_array_elements()->end(); ++it) {
      HexifyPtrRecursive(&*it);
    }
  } else {
    PJCORE_CHECK_EQ(JsonValue::TYPE_UNSIGNED, value->type());

    *value = MakeJsonValue(HexifyPtr(value->unsigned_value()));
  }
}

void LiveCapturableList::ConsiderHexifyPtrRecursive(JsonValue* value) {
  PJCORE_CHECK(value);
  if (value->type() == JsonValue::TYPE_OBJECT) {
    for (JsonPropertyList::iterator it =
             value->mutable_object_properties()->begin();
         it != value->mutable_object_properties()->end(); ++it) {
      StringPiece name = it->name();
      if (name == "ptr" || name.ends_with("_ptr")) {
        HexifyPtrRecursive(it->mutable_value());
      } else {
        ConsiderHexifyPtrRecursive(it->mutable_value());
      }
    }
  } else if (value->type() == JsonValue::TYPE_ARRAY) {
    for (google::protobuf::RepeatedPtrField<JsonValue>::iterator it =
             value->mutable_array_elements()->begin();
         it != value->mutable_array_elements()->end(); ++it) {
      ConsiderHexifyPtrRecursive(&*it);
    }
  }
}

}  // namespace pjcore
