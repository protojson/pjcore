{
  'variables': {
  },

  'includes': [
  ],

  'targets': [
    {
      'target_name': 'gtest',
      'type': 'static_library',
      'include_dirs': [
        'external/gtest/include',
        'external/gtest',
      ],
      'sources': [
        'external/gtest/src/gtest-death-test.cc',
        'external/gtest/src/gtest-filepath.cc',
        'external/gtest/src/gtest-port.cc',
        'external/gtest/src/gtest-printers.cc',
        'external/gtest/src/gtest-test-part.cc',
        'external/gtest/src/gtest-typed-test.cc',
        'external/gtest/src/gtest.cc',
      ],
    },

    {
      'target_name': 'gmock',
      'type': 'static_library',
      'include_dirs': [
        'external/gtest/include',
        'external/gmock/include',
      ],
      'sources': [
        'external/gmock/src/gmock-cardinalities.cc',
        'external/gmock/src/gmock-internal-utils.cc',
        'external/gmock/src/gmock-matchers.cc',
        'external/gmock/src/gmock-spec-builders.cc',
        'external/gmock/src/gmock.cc',
        'external/gmock/src/gmock_main.cc',
      ],
    },

    {
      'target_name': 'protobuf',
      'type': 'static_library',
      'include_dirs': [
        'external/protobuf/src',
        'external/fixes/<(OS)',
      ],
      'defines': [
        'GOOGLE_PROTOBUF_NO_RTTI',
      ],
      'all_dependent_settings': {
        'defines': [
          'GOOGLE_PROTOBUF_NO_RTTI',
        ],
      },
      'conditions': [
        ['OS=="win"', {
          'defines': [
            '__thread=__declspec(thread)',
          ],
          'msvs_settings': {
            'VCCLCompilerTool': {
              'ForcedIncludeFiles': ['fix_type_traits.h'],
            },
          },
          'all_dependent_settings': {
            'include_dirs': [
              'external/fixes/<(OS)',
            ],
            'defines': [
              '__thread=__declspec(thread)',
            ],
            'msvs_settings': {
              'VCCLCompilerTool': {
                'ForcedIncludeFiles': ['fix_type_traits.h'],
              },
            },
          },
        }],
        ['OS=="linux"', {
          'defines': [
            'va_copy=__va_copy',
          ],
        }],
      ],
      'sources': [
        'external/protobuf/src/google/protobuf/arena.cc',
        'external/protobuf/src/google/protobuf/arenastring.cc',
        'external/protobuf/src/google/protobuf/compiler/code_generator.cc',
        'external/protobuf/src/google/protobuf/compiler/command_line_interface.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_enum.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_enum_field.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_extension.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_field.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_file.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_generator.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_helpers.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_map_field.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_message.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_message_field.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_primitive_field.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_service.cc',
        'external/protobuf/src/google/protobuf/compiler/cpp/cpp_string_field.cc',
        'external/protobuf/src/google/protobuf/compiler/importer.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_context.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_doc_comment.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_enum.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_enum_field.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_extension.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_field.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_file.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_generator.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_generator_factory.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_helpers.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_lazy_message_field.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_map_field.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_message.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_message_field.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_name_resolver.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_primitive_field.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_service.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_shared_code_generator.cc',
        'external/protobuf/src/google/protobuf/compiler/java/java_string_field.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_enum.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_enum_field.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_extension.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_field.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_file.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_generator.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_helpers.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_message.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_message_field.cc',
        'external/protobuf/src/google/protobuf/compiler/javanano/javanano_primitive_field.cc',
        'external/protobuf/src/google/protobuf/compiler/parser.cc',
        'external/protobuf/src/google/protobuf/compiler/plugin.cc',
        'external/protobuf/src/google/protobuf/compiler/plugin.pb.cc',
        'external/protobuf/src/google/protobuf/compiler/python/python_generator.cc',
        'external/protobuf/src/google/protobuf/compiler/subprocess.cc',
        'external/protobuf/src/google/protobuf/compiler/zip_writer.cc',
        'external/protobuf/src/google/protobuf/descriptor.cc',
        'external/protobuf/src/google/protobuf/descriptor.pb.cc',
        'external/protobuf/src/google/protobuf/descriptor_database.cc',
        'external/protobuf/src/google/protobuf/dynamic_message.cc',
        'external/protobuf/src/google/protobuf/extension_set.cc',
        'external/protobuf/src/google/protobuf/extension_set_heavy.cc',
        'external/protobuf/src/google/protobuf/generated_message_reflection.cc',
        'external/protobuf/src/google/protobuf/generated_message_util.cc',
        'external/protobuf/src/google/protobuf/io/coded_stream.cc',
        'external/protobuf/src/google/protobuf/io/gzip_stream.cc',
        'external/protobuf/src/google/protobuf/io/printer.cc',
        'external/protobuf/src/google/protobuf/io/strtod.cc',
        'external/protobuf/src/google/protobuf/io/tokenizer.cc',
        'external/protobuf/src/google/protobuf/io/zero_copy_stream.cc',
        'external/protobuf/src/google/protobuf/io/zero_copy_stream_impl.cc',
        'external/protobuf/src/google/protobuf/io/zero_copy_stream_impl_lite.cc',
        'external/protobuf/src/google/protobuf/map_field.cc',
        'external/protobuf/src/google/protobuf/message.cc',
        'external/protobuf/src/google/protobuf/message_lite.cc',
        'external/protobuf/src/google/protobuf/reflection_ops.cc',
        'external/protobuf/src/google/protobuf/repeated_field.cc',
        'external/protobuf/src/google/protobuf/service.cc',
        'external/protobuf/src/google/protobuf/stubs/atomicops_internals_x86_gcc.cc',
        'external/protobuf/src/google/protobuf/stubs/atomicops_internals_x86_msvc.cc',
        'external/protobuf/src/google/protobuf/stubs/common.cc',
        'external/protobuf/src/google/protobuf/stubs/once.cc',
        'external/protobuf/src/google/protobuf/stubs/stringprintf.cc',
        'external/protobuf/src/google/protobuf/stubs/structurally_valid.cc',
        'external/protobuf/src/google/protobuf/stubs/strutil.cc',
        'external/protobuf/src/google/protobuf/stubs/substitute.cc',
        'external/protobuf/src/google/protobuf/text_format.cc',
        'external/protobuf/src/google/protobuf/unknown_field_set.cc',
        'external/protobuf/src/google/protobuf/wire_format.cc',
        'external/protobuf/src/google/protobuf/wire_format_lite.cc',
      ],
    },

    {
      'target_name': 'protoc',
      'type': 'executable',
      'include_dirs': [
        'external/protobuf/src',
      ],
      'dependencies': [
        'protobuf',
      ],
      'sources': [
        'external/protobuf/src/google/protobuf/compiler/main.cc',
      ],
    },

    {
      'target_name': 'pjcore',
      'type': 'static_library',
      'dependencies': [
        'protobuf',
      ],
      'include_dirs': [
        'include',
        'src',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'conditions': [
        ['OS=="linux"', {
          'defines': [
            'PJCORE_DISTINCT_LONG_LONG',
          ],
        }],
      ],
      'sources': [
        'src/pjcore/abstract_counter.cc',
        'src/pjcore/abstract_http_client_transaction.cc',
        'src/pjcore/abstract_http_handler.cc',
        'src/pjcore/abstract_http_parser.cc',
        'src/pjcore/abstract_http_server_connection.cc',
        'src/pjcore/abstract_http_server_core.cc',
        'src/pjcore/abstract_uv.cc',
        'src/pjcore/auto_callback.cc',
        'src/pjcore/errno_description.cc',
        'src/pjcore/error.pb.cc',
        'src/pjcore/error_util.cc',
        'src/pjcore/http.pb.cc',
        'src/pjcore/http_callback_handler.cc',
        'src/pjcore/http_client.cc',
        'src/pjcore/http_client_connection.cc',
        'src/pjcore/http_client_connection_group.cc',
        'src/pjcore/http_client_core.cc',
        'src/pjcore/http_client_transaction.cc',
        'src/pjcore/http_parser_plus_plus.cc',
        'src/pjcore/http_path_handler.cc',
        'src/pjcore/http_server.cc',
        'src/pjcore/http_server_connection.cc',
        'src/pjcore/http_server_core.cc',
        'src/pjcore/http_server_transaction.cc',
        'src/pjcore/http_util.cc',
        'src/pjcore/idle_logger.cc',
        'src/pjcore/json_properties.cc',
        'src/pjcore/json_reader.cc',
        'src/pjcore/json_util.cc',
        'src/pjcore/json.pb.cc',
        'src/pjcore/json_writer.cc',
        'src/pjcore/live.pb.cc',
        'src/pjcore/live_addr_info.pb.cc',
        'src/pjcore/live_capturable.cc',
        'src/pjcore/live_http.pb.cc',
        'src/pjcore/live_util.cc',
        'src/pjcore/live_uv.pb.cc',
        'src/pjcore/logging.cc',
        'src/pjcore/make_json_value.cc',
        'src/pjcore/make_json_value_message.cc',
        'src/pjcore/number_util.cc',
        'src/pjcore/shared_addr_info_list.cc',
        'src/pjcore/shared_future.cc',
        'src/pjcore/shared_uv_loop.cc',
        'src/pjcore/string_piece_util.cc',
        'src/pjcore/text_location.cc',
        'src/pjcore/third_party/chromium/atomicops_internals_x86_gcc.cc',
        'src/pjcore/third_party/chromium/bind_helpers.cc',
        'src/pjcore/third_party/chromium/callback_helpers.cc',
        'src/pjcore/third_party/chromium/callback_internal.cc',
        'src/pjcore/third_party/chromium/ref_counted.cc',
        'src/pjcore/third_party/chromium/string_piece.cc',
        'src/pjcore/third_party/chromium/weak_ptr.cc',
        'src/pjcore/third_party/gtest/are_almost_equal.cc',
        'src/pjcore/unbox_json_value.cc',
        'src/pjcore/unbox_json_value_message.cc',
        'src/pjcore/unicode.cc',
        'src/pjcore/url_util.cc',
        'src/pjcore/uv_util.cc',
      ],
    },

    {
      'target_name': 'pjcore_test',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
        'gmock',
        'gtest',
      ],
      'include_dirs': [
        'include',
        'src',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
        'external/gtest/include',
        'external/gmock/include',
      ],
      'sources': [
        'src/pjcore_test/abstract_counter_test.cc',
        'src/pjcore_test/auto_callback_test.cc',
        'src/pjcore_test/base_64_test.cc',
        'src/pjcore_test/http_client_connection_test.cc',
        'src/pjcore_test/http_client_transaction_test.cc',
        'src/pjcore_test/http_parser_plus_plus_test.cc',
        'src/pjcore_test/http_parser_test_message.cc',
        'src/pjcore_test/http_path_handler_test.cc',
        'src/pjcore_test/http_server_connection_test.cc',
        'src/pjcore_test/http_server_core_test.cc',
        'src/pjcore_test/http_server_test.cc',
        'src/pjcore_test/http_server_transaction_test.cc',
        'src/pjcore_test/json_properties_test.cc',
        'src/pjcore_test/json_reader_test.cc',
        'src/pjcore_test/json_util_test.cc',
        'src/pjcore_test/json_writer_test.cc',
        'src/pjcore_test/live_capturable_test.cc',
        'src/pjcore_test/logging_test.cc',
        'src/pjcore_test/make_json_value_message_test.cc',
        'src/pjcore_test/make_json_value_test.cc',
        'src/pjcore_test/mock_http_client_transaction.cc',
        'src/pjcore_test/mock_http_handler.cc',
        'src/pjcore_test/mock_http_parser.cc',
        'src/pjcore_test/mock_http_server_connection.cc',
        'src/pjcore_test/mock_http_server_core.cc',
        'src/pjcore_test/mock_uv_base.cc',
        'src/pjcore_test/name_value_util_test.cc',
        'src/pjcore_test/number_util_test.cc',
        'src/pjcore_test/parse_url_test.cc',
        'src/pjcore_test/shared_uv_loop_test.cc',
        'src/pjcore_test/test_message.pb.cc',
        'src/pjcore_test/text_location_test.cc',
        'src/pjcore_test/unbox_json_value_message_test.cc',
        'src/pjcore_test/unbox_json_value_test.cc',
        'src/pjcore_test/url_parser_test_message.cc',
      ],
    },

    {
      'target_name': 'benchmark_server',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
      ],
      'include_dirs': [
        'include',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'sources': [
        'benchmark/server/server.cc',
      ],
    },

    {
      'target_name': 'benchmark_client',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
      ],
      'include_dirs': [
        'include',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'sources': [
        'benchmark/client/client.cc',
      ],
    },

    {
      'target_name': 'use_case_output_json',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
      ],
      'include_dirs': [
        'include',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'sources': [
        'use_cases/output_json/business.pb.cc',
        'use_cases/output_json/output_json.cc',
      ],
    },

    {
      'target_name': 'use_case_output_pretty_json',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
      ],
      'include_dirs': [
        'include',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'sources': [
        'use_cases/output_pretty_json/business.pb.cc',
        'use_cases/output_pretty_json/output_pretty_json.cc',
      ],
    },

    {
      'target_name': 'use_case_parse_json',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
      ],
      'include_dirs': [
        'include',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'sources': [
        'use_cases/parse_json/article.pb.cc',
        'use_cases/parse_json/parse_json.cc',
      ],
    },

    {
      'target_name': 'use_case_parse_unknown_json',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
      ],
      'include_dirs': [
        'include',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'sources': [
        'use_cases/parse_unknown_json/review.pb.cc',
        'use_cases/parse_unknown_json/parse_unknown_json.cc',
      ],
    },

    {
      'target_name': 'use_case_fetch_url',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
      ],
      'include_dirs': [
        'include',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'sources': [
        'use_cases/fetch_url/fetch_url.cc',
      ],
    },

    {
      'target_name': 'use_case_web_server',
      'type': 'executable',
      'dependencies': [
        'pjcore',
        'protobuf',
        'external/libuv/uv.gyp:libuv',
        'external/http-parser/http_parser.gyp:http_parser',
      ],
      'include_dirs': [
        'include',
        'external/protobuf/src',
        'external/libuv/include',
        'external/http-parser',
      ],
      'sources': [
        'use_cases/web_server/web_server.cc',
      ],
    },

  ],
}
