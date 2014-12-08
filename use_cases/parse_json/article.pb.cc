// Generated by the protocol buffer compiler.  DO NOT EDIT!
// source: article.proto

#define INTERNAL_SUPPRESS_PROTOBUF_FIELD_DEPRECATION
#include "article.pb.h"

#include <algorithm>

#include <google/protobuf/stubs/common.h>
#include <google/protobuf/stubs/once.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/wire_format_lite_inl.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/generated_message_reflection.h>
#include <google/protobuf/reflection_ops.h>
#include <google/protobuf/wire_format.h>
// @@protoc_insertion_point(includes)

namespace {

const ::google::protobuf::Descriptor* Article_descriptor_ = NULL;
const ::google::protobuf::internal::GeneratedMessageReflection*
  Article_reflection_ = NULL;

}  // namespace


void protobuf_AssignDesc_article_2eproto() {
  protobuf_AddDesc_article_2eproto();
  const ::google::protobuf::FileDescriptor* file =
    ::google::protobuf::DescriptorPool::generated_pool()->FindFileByName(
      "article.proto");
  GOOGLE_CHECK(file != NULL);
  Article_descriptor_ = file->message_type(0);
  static const int Article_offsets_[2] = {
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Article, title_),
    GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Article, content_),
  };
  Article_reflection_ =
    ::google::protobuf::internal::GeneratedMessageReflection::NewGeneratedMessageReflection(
      Article_descriptor_,
      Article::default_instance_,
      Article_offsets_,
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Article, _has_bits_[0]),
      -1,
      -1,
      sizeof(Article),
      GOOGLE_PROTOBUF_GENERATED_MESSAGE_FIELD_OFFSET(Article, _internal_metadata_));
}

namespace {

GOOGLE_PROTOBUF_DECLARE_ONCE(protobuf_AssignDescriptors_once_);
inline void protobuf_AssignDescriptorsOnce() {
  ::google::protobuf::GoogleOnceInit(&protobuf_AssignDescriptors_once_,
                 &protobuf_AssignDesc_article_2eproto);
}

void protobuf_RegisterTypes(const ::std::string&) {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedMessage(
      Article_descriptor_, &Article::default_instance());
}

}  // namespace

void protobuf_ShutdownFile_article_2eproto() {
  delete Article::default_instance_;
  delete Article_reflection_;
}

void protobuf_AddDesc_article_2eproto() {
  static bool already_here = false;
  if (already_here) return;
  already_here = true;
  GOOGLE_PROTOBUF_VERIFY_VERSION;

  ::google::protobuf::DescriptorPool::InternalAddGeneratedFile(
    "\n\rarticle.proto\")\n\007Article\022\r\n\005title\030\001 \001("
    "\t\022\017\n\007content\030\002 \001(\t", 58);
  ::google::protobuf::MessageFactory::InternalRegisterGeneratedFile(
    "article.proto", &protobuf_RegisterTypes);
  Article::default_instance_ = new Article();
  Article::default_instance_->InitAsDefaultInstance();
  ::google::protobuf::internal::OnShutdown(&protobuf_ShutdownFile_article_2eproto);
}

// Force AddDescriptors() to be called at static initialization time.
struct StaticDescriptorInitializer_article_2eproto {
  StaticDescriptorInitializer_article_2eproto() {
    protobuf_AddDesc_article_2eproto();
  }
} static_descriptor_initializer_article_2eproto_;

namespace {

static void MergeFromFail(int line) GOOGLE_ATTRIBUTE_COLD;
static void MergeFromFail(int line) {
  GOOGLE_CHECK(false) << __FILE__ << ":" << line;
}

}  // namespace


// ===================================================================

#ifndef _MSC_VER
const int Article::kTitleFieldNumber;
const int Article::kContentFieldNumber;
#endif  // !_MSC_VER

Article::Article()
  : ::google::protobuf::Message() , _internal_metadata_(NULL)  {
  SharedCtor();
  // @@protoc_insertion_point(constructor:Article)
}

void Article::InitAsDefaultInstance() {
}

Article::Article(const Article& from)
  : ::google::protobuf::Message(),
    _internal_metadata_(NULL) {
  SharedCtor();
  MergeFrom(from);
  // @@protoc_insertion_point(copy_constructor:Article)
}

void Article::SharedCtor() {
  ::google::protobuf::internal::GetEmptyString();
  _cached_size_ = 0;
  title_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  content_.UnsafeSetDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
}

Article::~Article() {
  // @@protoc_insertion_point(destructor:Article)
  SharedDtor();
}

void Article::SharedDtor() {
  title_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  content_.DestroyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
  if (this != default_instance_) {
  }
}

void Article::SetCachedSize(int size) const {
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
}
const ::google::protobuf::Descriptor* Article::descriptor() {
  protobuf_AssignDescriptorsOnce();
  return Article_descriptor_;
}

const Article& Article::default_instance() {
  if (default_instance_ == NULL) protobuf_AddDesc_article_2eproto();
  return *default_instance_;
}

Article* Article::default_instance_ = NULL;

Article* Article::New(::google::protobuf::Arena* arena) const {
  Article* n = new Article;
  if (arena != NULL) {
    arena->Own(n);
  }
  return n;
}

void Article::Clear() {
  if (_has_bits_[0 / 32] & 3) {
    if (has_title()) {
      title_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    }
    if (has_content()) {
      content_.ClearToEmptyNoArena(&::google::protobuf::internal::GetEmptyStringAlreadyInited());
    }
  }
  ::memset(_has_bits_, 0, sizeof(_has_bits_));
  if (_internal_metadata_.have_unknown_fields()) {
    mutable_unknown_fields()->Clear();
  }
}

bool Article::MergePartialFromCodedStream(
    ::google::protobuf::io::CodedInputStream* input) {
#define DO_(EXPRESSION) if (!(EXPRESSION)) goto failure
  ::google::protobuf::uint32 tag;
  // @@protoc_insertion_point(parse_start:Article)
  for (;;) {
    ::std::pair< ::google::protobuf::uint32, bool> p = input->ReadTagWithCutoff(127);
    tag = p.first;
    if (!p.second) goto handle_unusual;
    switch (::google::protobuf::internal::WireFormatLite::GetTagFieldNumber(tag)) {
      // optional string title = 1;
      case 1: {
        if (tag == 10) {
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_title()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->title().data(), this->title().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "Article.title");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectTag(18)) goto parse_content;
        break;
      }

      // optional string content = 2;
      case 2: {
        if (tag == 18) {
         parse_content:
          DO_(::google::protobuf::internal::WireFormatLite::ReadString(
                input, this->mutable_content()));
          ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
            this->content().data(), this->content().length(),
            ::google::protobuf::internal::WireFormat::PARSE,
            "Article.content");
        } else {
          goto handle_unusual;
        }
        if (input->ExpectAtEnd()) goto success;
        break;
      }

      default: {
      handle_unusual:
        if (tag == 0 ||
            ::google::protobuf::internal::WireFormatLite::GetTagWireType(tag) ==
            ::google::protobuf::internal::WireFormatLite::WIRETYPE_END_GROUP) {
          goto success;
        }
        DO_(::google::protobuf::internal::WireFormat::SkipField(
              input, tag, mutable_unknown_fields()));
        break;
      }
    }
  }
success:
  // @@protoc_insertion_point(parse_success:Article)
  return true;
failure:
  // @@protoc_insertion_point(parse_failure:Article)
  return false;
#undef DO_
}

void Article::SerializeWithCachedSizes(
    ::google::protobuf::io::CodedOutputStream* output) const {
  // @@protoc_insertion_point(serialize_start:Article)
  // optional string title = 1;
  if (has_title()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->title().data(), this->title().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "Article.title");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      1, this->title(), output);
  }

  // optional string content = 2;
  if (has_content()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->content().data(), this->content().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "Article.content");
    ::google::protobuf::internal::WireFormatLite::WriteStringMaybeAliased(
      2, this->content(), output);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    ::google::protobuf::internal::WireFormat::SerializeUnknownFields(
        unknown_fields(), output);
  }
  // @@protoc_insertion_point(serialize_end:Article)
}

::google::protobuf::uint8* Article::SerializeWithCachedSizesToArray(
    ::google::protobuf::uint8* target) const {
  // @@protoc_insertion_point(serialize_to_array_start:Article)
  // optional string title = 1;
  if (has_title()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->title().data(), this->title().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "Article.title");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        1, this->title(), target);
  }

  // optional string content = 2;
  if (has_content()) {
    ::google::protobuf::internal::WireFormat::VerifyUTF8StringNamedField(
      this->content().data(), this->content().length(),
      ::google::protobuf::internal::WireFormat::SERIALIZE,
      "Article.content");
    target =
      ::google::protobuf::internal::WireFormatLite::WriteStringToArray(
        2, this->content(), target);
  }

  if (_internal_metadata_.have_unknown_fields()) {
    target = ::google::protobuf::internal::WireFormat::SerializeUnknownFieldsToArray(
        unknown_fields(), target);
  }
  // @@protoc_insertion_point(serialize_to_array_end:Article)
  return target;
}

int Article::ByteSize() const {
  int total_size = 0;

  if (_has_bits_[0 / 32] & 3) {
    // optional string title = 1;
    if (has_title()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->title());
    }

    // optional string content = 2;
    if (has_content()) {
      total_size += 1 +
        ::google::protobuf::internal::WireFormatLite::StringSize(
          this->content());
    }

  }
  if (_internal_metadata_.have_unknown_fields()) {
    total_size +=
      ::google::protobuf::internal::WireFormat::ComputeUnknownFieldsSize(
        unknown_fields());
  }
  GOOGLE_SAFE_CONCURRENT_WRITES_BEGIN();
  _cached_size_ = total_size;
  GOOGLE_SAFE_CONCURRENT_WRITES_END();
  return total_size;
}

void Article::MergeFrom(const ::google::protobuf::Message& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  const Article* source =
    ::google::protobuf::internal::dynamic_cast_if_available<const Article*>(
      &from);
  if (source == NULL) {
    ::google::protobuf::internal::ReflectionOps::Merge(from, this);
  } else {
    MergeFrom(*source);
  }
}

void Article::MergeFrom(const Article& from) {
  if (GOOGLE_PREDICT_FALSE(&from == this)) MergeFromFail(__LINE__);
  if (from._has_bits_[0 / 32] & (0xffu << (0 % 32))) {
    if (from.has_title()) {
      set_has_title();
      title_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.title_);
    }
    if (from.has_content()) {
      set_has_content();
      content_.AssignWithDefault(&::google::protobuf::internal::GetEmptyStringAlreadyInited(), from.content_);
    }
  }
  if (from._internal_metadata_.have_unknown_fields()) {
    mutable_unknown_fields()->MergeFrom(from.unknown_fields());
  }
}

void Article::CopyFrom(const ::google::protobuf::Message& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

void Article::CopyFrom(const Article& from) {
  if (&from == this) return;
  Clear();
  MergeFrom(from);
}

bool Article::IsInitialized() const {

  return true;
}

void Article::Swap(Article* other) {
  if (other == this) return;
  InternalSwap(other);
}
void Article::InternalSwap(Article* other) {
  title_.Swap(&other->title_);
  content_.Swap(&other->content_);
  std::swap(_has_bits_[0], other->_has_bits_[0]);
  _internal_metadata_.Swap(&other->_internal_metadata_);
  std::swap(_cached_size_, other->_cached_size_);
}

::google::protobuf::Metadata Article::GetMetadata() const {
  protobuf_AssignDescriptorsOnce();
  ::google::protobuf::Metadata metadata;
  metadata.descriptor = Article_descriptor_;
  metadata.reflection = Article_reflection_;
  return metadata;
}


// @@protoc_insertion_point(namespace_scope)

// @@protoc_insertion_point(global_scope)
