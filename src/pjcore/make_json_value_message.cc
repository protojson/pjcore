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

#include "pjcore/make_json_value.h"

#include "pjcore/logging.h"
#include "pjcore/json_util.h"
#include "pjcore/number_util.h"
#include "pjcore/string_piece_util.h"

#define OBJECT_PROPERTIES_STR "object_properties"

#ifdef GetMessage
#undef GetMessage
#endif

namespace pjcore {

using google::protobuf::Descriptor;
using google::protobuf::EnumValueDescriptor;
using google::protobuf::FieldDescriptor;
using google::protobuf::Message;
using google::protobuf::Reflection;
using google::protobuf::RepeatedPtrField;

namespace {

void MakeJsonValueOut(const Message& message, JsonValue* value);

void FieldToJsonOut(const Message& message, const Reflection& reflection,
                    const FieldDescriptor& field, JsonValue* value) {
  PJCORE_CHECK(value);

  if (!field.is_repeated()) {
    switch (field.cpp_type()) {
      case FieldDescriptor::CPPTYPE_INT32:  // TYPE_INT32, TYPE_SINT32,
                                            // TYPE_SFIXED32
        *value = MakeJsonValue(reflection.GetInt32(message, &field));
        break;

      case FieldDescriptor::CPPTYPE_INT64:  // TYPE_INT64, TYPE_SINT64,
                                            // TYPE_SFIXED64
        *value = MakeJsonValue(reflection.GetInt64(message, &field));
        break;

      case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
        *value = MakeJsonValue(reflection.GetUInt32(message, &field));
        break;

      case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
        *value = MakeJsonValue(reflection.GetUInt64(message, &field));
        break;

      case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
        *value = MakeJsonValue(reflection.GetDouble(message, &field));
        break;

      case FieldDescriptor::CPPTYPE_FLOAT:  // TYPE_FLOAT
        *value = MakeJsonValue(reflection.GetFloat(message, &field));
        break;

      case FieldDescriptor::CPPTYPE_BOOL:  // TYPE_BOOL
        *value = MakeJsonValue(reflection.GetBool(message, &field));
        break;

      case FieldDescriptor::CPPTYPE_ENUM:  // TYPE_ENUM
        *value = MakeJsonValue(reflection.GetEnum(message, &field)->name());
        break;

      case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
        if (field.type() == FieldDescriptor::TYPE_STRING) {
          *value = MakeJsonValue(reflection.GetString(message, &field));
        } else {
          *value =
              MakeJsonValue(WriteBase64(reflection.GetString(message, &field)));
        }
        break;

      case FieldDescriptor::CPPTYPE_MESSAGE:  // TYPE_MESSAGE, TYPE_GROUP
        *value = MakeJsonValue(reflection.GetMessage(message, &field));
        break;

      default:
        PJCORE_CHECK(false);  // fied.cpp_type()
    }
  } else {
    int field_size = reflection.FieldSize(message, &field);
    value->set_type(JsonValue::TYPE_ARRAY);
    value->mutable_array_elements()->Reserve(field_size);
    for (int field_index = 0; field_index < field_size; ++field_index) {
      switch (field.cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:  // TYPE_INT32, TYPE_SINT32,
                                              // TYPE_SFIXED32
          *value->add_array_elements() = MakeJsonValue(
              reflection.GetRepeatedInt32(message, &field, field_index));
          break;

        case FieldDescriptor::CPPTYPE_INT64:  // TYPE_INT64, TYPE_SINT64,
                                              // TYPE_SFIXED64
          *value->add_array_elements() = MakeJsonValue(
              reflection.GetRepeatedInt64(message, &field, field_index));
          break;

        case FieldDescriptor::CPPTYPE_UINT32:  // TYPE_UINT32, TYPE_FIXED32
          *value->add_array_elements() = MakeJsonValue(
              reflection.GetRepeatedUInt32(message, &field, field_index));
          break;

        case FieldDescriptor::CPPTYPE_UINT64:  // TYPE_UINT64, TYPE_FIXED64
          *value->add_array_elements() = MakeJsonValue(
              reflection.GetRepeatedUInt64(message, &field, field_index));
          break;

        case FieldDescriptor::CPPTYPE_DOUBLE:  // TYPE_DOUBLE
          *value->add_array_elements() = MakeJsonValue(
              reflection.GetRepeatedDouble(message, &field, field_index));
          break;

        case FieldDescriptor::CPPTYPE_FLOAT:  // TYPE_FLOAT
          *value->add_array_elements() = MakeJsonValue(
              reflection.GetRepeatedFloat(message, &field, field_index));
          break;

        case FieldDescriptor::CPPTYPE_BOOL:  // TYPE_BOOL
          *value->add_array_elements() = MakeJsonValue(
              reflection.GetRepeatedBool(message, &field, field_index));
          break;

        case FieldDescriptor::CPPTYPE_ENUM:  // TYPE_ENUM
          *value->add_array_elements() = MakeJsonValue(
              reflection.GetRepeatedEnum(message, &field, field_index)->name());
          break;

        case FieldDescriptor::CPPTYPE_STRING:  // TYPE_STRING, TYPE_BYTES
          if (field.type() == FieldDescriptor::TYPE_STRING) {
            *value->add_array_elements() = MakeJsonValue(
                reflection.GetRepeatedString(message, &field, field_index));
          } else {
            *value->add_array_elements() = MakeJsonValue(WriteBase64(
                reflection.GetRepeatedString(message, &field, field_index)));
          }
          break;

        case FieldDescriptor::CPPTYPE_MESSAGE:  // TYPE_MESSAGE, TYPE_GROUP
          MakeJsonValueOut(
              reflection.GetRepeatedMessage(message, &field, field_index),
              value->add_array_elements());
          break;

        default:
          PJCORE_CHECK(false);  // fied.cpp_type()
      }
    }
  }
}

void MakeJsonValueOut(const Message& message, JsonValue* value) {
  PJCORE_CHECK(value);
  value->Clear();

  const Descriptor* descriptor = message.GetDescriptor();

  if (descriptor == JsonValue::descriptor()) {
    value->CopyFrom(message);
    return;
  }

  value->set_type(JsonValue::TYPE_OBJECT);

  const Reflection& reflection = *message.GetReflection();

  for (int field_index = 0; field_index < descriptor->field_count();
       ++field_index) {
    const FieldDescriptor& field = *descriptor->field(field_index);

    if (field.is_repeated() ? !reflection.FieldSize(message, &field)
                            : !reflection.HasField(message, &field)) {
      // empty.
      continue;
    }

    if (field.is_repeated() &&
        field.cpp_type() == FieldDescriptor::CPPTYPE_MESSAGE &&
        field.message_type() == JsonValue::Property::descriptor() &&
        StringPiece(field.name()) == StringPiece(OBJECT_PROPERTIES_STR)) {
      int field_size = reflection.FieldSize(message, &field);
      for (int index = 0; index < field_size; ++index) {
        *value->add_object_properties() =
            static_cast<const JsonValue::Property&>(
                reflection.GetRepeatedMessage(message, &field, index));
      }
    } else {
      JsonValue::Property* property = value->add_object_properties();

      property->set_name(field.name());

      FieldToJsonOut(message, reflection, field, property->mutable_value());
    }
  }
}

}  // unnamed namespace

JsonValue MakeJsonValue(const Message& message) {
  JsonValue value;
  MakeJsonValueOut(message, &value);
  return value;
}

}  // namespace pjcore
