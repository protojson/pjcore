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

#include "pjcore/unbox_json_value.h"

#include "pjcore/logging.h"
#include "pjcore/make_json_value.h"
#include "pjcore/number_util.h"
#include "pjcore/string_piece_util.h"

#define OBJECT_PROPERTIES_STR "object_properties"

namespace pjcore {

using google::protobuf::Descriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;
using google::protobuf::RepeatedPtrField;

namespace {

bool UnboxEnum(const JsonValue& json_value, const FieldDescriptor& field,
               const google::protobuf::EnumValueDescriptor** enum_value,
               Error* error) {
  PJCORE_CHECK(error);
  error->Clear();

  PJCORE_CHECK(enum_value);
  *enum_value = NULL;

  if (json_value.type() == JsonValue::TYPE_STRING) {
    *enum_value = field.enum_type()->FindValueByName(json_value.string_value());
    if (*enum_value) {
      return true;
    }
    int32_t number;
    if (ReadNumber(json_value.string_value(), &number)) {
      *enum_value = field.enum_type()->FindValueByNumber(number);
      return true;
    }

    *enum_value = NULL;
    return true;
  } else {
    int32_t number;
    PJCORE_REQUIRE(UnboxJsonValue(json_value, &number, error),
                   "Invalid enum json_value");

    *enum_value = field.enum_type()->FindValueByNumber(number);
    return true;
  }
}

bool UnboxField(const JsonValue& json_value, const Reflection& reflection,
                const FieldDescriptor& field,
                google::protobuf::Message* message, Error* error,
                google::protobuf::RepeatedPtrField<JsonValue::Property>*
                    unknown_object_properties) {
  PJCORE_CHECK(message);
  PJCORE_CHECK(error);

  if (json_value.type() == JsonValue::TYPE_NULL) {
    reflection.ClearField(message, &field);
    return true;
  }

  if (!field.is_repeated()) {
    switch (field.cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32:  // TYPE_INT32, TYPE_SINT32,
                                            // TYPE_SFIXED32
        {
          int32_t unboxed;
          PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &unboxed, error),
                                "Failed to unbox int32_t");
          reflection.SetInt32(message, &field, unboxed);
        }
        break;

      case FieldDescriptor::CPPTYPE_INT64:  // TYPE_INT64, TYPE_SINT64,
                                            // TYPE_SFIXED64
        {
          int64_t unboxed;
          PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &unboxed, error),
                                "Failed to unbox int64_t");
          reflection.SetInt64(message, &field, unboxed);
        }
        break;

      case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
      {
        uint32_t unboxed;
        PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &unboxed, error),
                              "Failed to unbox uint32_t");
        reflection.SetUInt32(message, &field, unboxed);
      } break;

      case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
      {
        uint64_t unboxed;
        PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &unboxed, error),
                              "Failed to unbox uint64_t");
        reflection.SetUInt64(message, &field, unboxed);
      } break;

      case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
      {
        double unboxed;
        PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &unboxed, error),
                              "Failed to unbox double");
        reflection.SetDouble(message, &field, unboxed);
      } break;

      case FieldDescriptor::CPPTYPE_FLOAT:  // TYPE_FLOAT
      {
        float unboxed;
        PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &unboxed, error),
                              "Failed to unbox float");
        reflection.SetFloat(message, &field, unboxed);
      } break;

      case FieldDescriptor::CPPTYPE_BOOL:  // TYPE_BOOL
      {
        bool unboxed;
        PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &unboxed, error),
                              "Failed to unbox bool");
        reflection.SetBool(message, &field, unboxed);
      } break;

      case FieldDescriptor::CPPTYPE_ENUM:  // TYPE_ENUM
      {
        const google::protobuf::EnumValueDescriptor* unboxed;
        PJCORE_REQUIRE_SILENT(UnboxEnum(json_value, field, &unboxed, error),
                              "Failed to unbox enum");
        if (unboxed) {
          reflection.SetEnum(message, &field, unboxed);
        }
        break;
      }

      case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
      {
        std::string unboxed;
        PJCORE_REQUIRE_SILENT(UnboxJsonValue(json_value, &unboxed, error),
                              "Failed to unbox string");
        if (field.type() == FieldDescriptor::TYPE_STRING) {
          reflection.SetString(message, &field, unboxed);
        } else {
          std::string bytes;
          PJCORE_REQUIRE_SILENT(ReadBase64(unboxed, &bytes, error),
                                "Invalid Base64");
          reflection.SetString(message, &field, bytes);
        }
      } break;

      case FieldDescriptor::CPPTYPE_MESSAGE:  // TYPE_MESSAGE, TYPE_GROUP
        PJCORE_REQUIRE(UnboxJsonValue(json_value, reflection.MutableMessage(
                                                      message, &field),
                                      error, unknown_object_properties),
                       "Failed to unbox message");
        break;

      default:
        PJCORE_CHECK(false);  // field.cpp_type()
    }
  } else {
    if (json_value.type() == JsonValue::TYPE_ARRAY) {
      for (google::protobuf::RepeatedPtrField<JsonValue>::const_iterator it =
               json_value.array_elements().begin();
           it != json_value.array_elements().end(); ++it) {
        switch (field.cpp_type()) {
          case FieldDescriptor::CPPTYPE_INT32:  // TYPE_INT32, TYPE_SINT32,
                                                // TYPE_SFIXED32
            {
              int32_t unboxed;
              PJCORE_REQUIRE_SILENT(UnboxJsonValue(*it, &unboxed, error),
                                    "Failed to unbox int32_t");
              reflection.AddInt32(message, &field, unboxed);
            }
            break;

          case FieldDescriptor::CPPTYPE_INT64:  // TYPE_INT64, TYPE_SINT64,
                                                // TYPE_SFIXED64
            {
              int64_t unboxed;
              PJCORE_REQUIRE_SILENT(UnboxJsonValue(*it, &unboxed, error),
                                    "Failed to unbox int64_t");
              reflection.AddInt64(message, &field, unboxed);
            }
            break;

          case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
          {
            uint32_t unboxed;
            PJCORE_REQUIRE_SILENT(UnboxJsonValue(*it, &unboxed, error),
                                  "Failed to unbox uint32_t");
            reflection.AddUInt32(message, &field, unboxed);
          } break;

          case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
          {
            uint64_t unboxed;
            PJCORE_REQUIRE_SILENT(UnboxJsonValue(*it, &unboxed, error),
                                  "Failed to unbox uint64_t");
            reflection.AddUInt64(message, &field, unboxed);
          } break;

          case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
          {
            double unboxed;
            PJCORE_REQUIRE_SILENT(UnboxJsonValue(*it, &unboxed, error),
                                  "Failed to unbox double");
            reflection.AddDouble(message, &field, unboxed);
          } break;

          case FieldDescriptor::CPPTYPE_FLOAT:  // TYPE_FLOAT
          {
            float unboxed;
            PJCORE_REQUIRE_SILENT(UnboxJsonValue(*it, &unboxed, error),
                                  "Failed to unbox float");
            reflection.AddFloat(message, &field, unboxed);
          } break;

          case FieldDescriptor::CPPTYPE_BOOL:  // TYPE_BOOL
          {
            bool unboxed;
            PJCORE_REQUIRE_SILENT(UnboxJsonValue(*it, &unboxed, error),
                                  "Failed to unbox bool");
            reflection.AddBool(message, &field, unboxed);
          } break;

          case FieldDescriptor::CPPTYPE_ENUM:  // TYPE_ENUM
          {
            const google::protobuf::EnumValueDescriptor* unboxed;
            PJCORE_REQUIRE_SILENT(UnboxEnum(*it, field, &unboxed, error),
                                  "Failed to unbox enum");
            if (unboxed) {
              reflection.AddEnum(message, &field, unboxed);
            }
            break;
          }

          case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
          {
            std::string unboxed;
            PJCORE_REQUIRE_SILENT(UnboxJsonValue(*it, &unboxed, error),
                                  "Failed to unbox string");
            if (field.type() == FieldDescriptor::TYPE_STRING) {
              reflection.AddString(message, &field, unboxed);
            } else {
              std::string bytes;
              PJCORE_REQUIRE_SILENT(ReadBase64(unboxed, &bytes, error),
                                    "Invalid Base64");
              reflection.AddString(message, &field, bytes);
            }
          } break;

          case FieldDescriptor::CPPTYPE_MESSAGE:  // TYPE_MESSAGE, TYPE_GROUP
            PJCORE_REQUIRE(
                UnboxJsonValue(*it, reflection.AddMessage(message, &field),
                               error, unknown_object_properties),
                "Failed to unbox message");
            break;

          default:
            PJCORE_CHECK(false);  // field.cpp_type()
        }
      }
      return true;
    } else {
      const FieldDescriptor* repeated_name_field = NULL;

      if (json_value.type() == JsonValue::TYPE_OBJECT) {
        if (field.cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE) {
          repeated_name_field = field.message_type()->FindFieldByName("name");
        }
      }

      if (!repeated_name_field) {
        PJCORE_FAIL(
            "Array expected for repeated field, or object if repeated message "
            "has field name");
      }

      for (google::protobuf::RepeatedPtrField<
               JsonValue::Property>::const_iterator it =
               json_value.object_properties().begin();
           it != json_value.object_properties().end(); ++it) {
        google::protobuf::Message* target =
            reflection.AddMessage(message, &field);

        PJCORE_REQUIRE_SILENT(UnboxJsonValue(it->value(), target, error),
                              "Failed to parse message JSON");

        PJCORE_REQUIRE_SILENT(
            UnboxField(MakeJsonValue(it->name()), *target->GetReflection(),
                       *repeated_name_field, target, error,
                       unknown_object_properties),
            "Failed to parse field JSON");
      }
    }
    return true;
  }

  return true;
}

}  // unnamed namespace

bool UnboxJsonValue(const JsonValue& json_value,
                    google::protobuf::Message* message, Error* error,
                    google::protobuf::RepeatedPtrField<JsonValue::Property>*
                        unknown_object_properties) {
  PJCORE_CHECK(message);
  message->Clear();
  PJCORE_CHECK(error);
  error->Clear();

  const Descriptor* descriptor = message->GetDescriptor();

  if (descriptor == JsonValue::descriptor()) {
    message->CopyFrom(json_value);
    return true;
  }

  PJCORE_REQUIRE(json_value.type() == JsonValue::TYPE_OBJECT,
                 std::string("Value is not an object: ") +
                     JsonValue::Type_Name(json_value.type()));

  if (json_value.object_properties_size() == 0) {
    return true;
  }

  const Reflection& reflection = *message->GetReflection();

  bool considered_object_properties = false;
  const FieldDescriptor* object_properties_field = NULL;

  for (google::protobuf::RepeatedPtrField<JsonValue::Property>::const_iterator
           it = json_value.object_properties().begin();
       it != json_value.object_properties().end(); ++it) {
    const FieldDescriptor* field = descriptor->FindFieldByName(it->name());

    if (!field) {
      if (!considered_object_properties) {
        considered_object_properties = true;
        const FieldDescriptor* candidate =
            descriptor->FindFieldByName(OBJECT_PROPERTIES_STR);
        if (candidate) {
          if (candidate->is_repeated() &&
              candidate->cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
              candidate->message_type() == JsonValue::Property::descriptor()) {
            object_properties_field = candidate;
          }
        }
      }

      if (object_properties_field) {
        google::protobuf::Message* object_property =
            reflection.AddMessage(message, object_properties_field);
        PJCORE_CHECK(object_property->GetDescriptor() ==
                     JsonValue::Property::descriptor());

        static_cast<JsonValue::Property*>(object_property)->CopyFrom(*it);
      } else if (unknown_object_properties) {
        unknown_object_properties->Add()->CopyFrom(*it);
      }

      continue;
    }

    PJCORE_REQUIRE_SILENT(UnboxField(it->value(), reflection, *field, message,
                                     error, unknown_object_properties),
                          "Failed to parse field JSON");
  }

  return true;
}

}  // namespace pjcore
