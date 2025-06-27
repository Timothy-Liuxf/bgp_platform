# jsoncons
noinst_HEADERS +=                                                              \
    %reldir%/jsoncons/include/jsoncons/json_object.hpp                         \
    %reldir%/jsoncons/include/jsoncons/source_adaptor.hpp                      \
    %reldir%/jsoncons/include/jsoncons/staj_iterator.hpp                       \
    %reldir%/jsoncons/include/jsoncons/tag_type.hpp                            \
    %reldir%/jsoncons/include/jsoncons/byte_string.hpp                         \
    %reldir%/jsoncons/include/jsoncons/json_exception.hpp                      \
    %reldir%/jsoncons/include/jsoncons/json_type.hpp                           \
    %reldir%/jsoncons/include/jsoncons/conv_error.hpp                          \
    %reldir%/jsoncons/include/jsoncons/json_visitor.hpp                        \
    %reldir%/jsoncons/include/jsoncons/source.hpp                              \
    %reldir%/jsoncons/include/jsoncons/staj2_cursor.hpp                        \
    %reldir%/jsoncons/include/jsoncons/json_fwd.hpp                            \
    %reldir%/jsoncons/include/jsoncons/decode_json.hpp                         \
    %reldir%/jsoncons/include/jsoncons/uri.hpp                                 \
    %reldir%/jsoncons/include/jsoncons/unicode_traits.hpp                      \
    %reldir%/jsoncons/include/jsoncons/bigint.hpp                              \
    %reldir%/jsoncons/include/jsoncons/json_encoder.hpp                        \
    %reldir%/jsoncons/include/jsoncons/pretty_print.hpp                        \
    %reldir%/jsoncons/include/jsoncons/json_visitor2.hpp                       \
    %reldir%/jsoncons/include/jsoncons/sink.hpp                                \
    %reldir%/jsoncons/include/jsoncons/config/jsoncons_config.hpp              \
    %reldir%/jsoncons/include/jsoncons/config/binary_config.hpp                \
    %reldir%/jsoncons/include/jsoncons/config/compiler_support.hpp             \
    %reldir%/jsoncons/include/jsoncons/config/version.hpp                      \
    %reldir%/jsoncons/include/jsoncons/json_error.hpp                          \
    %reldir%/jsoncons/include/jsoncons/text_source_adaptor.hpp                 \
    %reldir%/jsoncons/include/jsoncons/json_options.hpp                        \
    %reldir%/jsoncons/include/jsoncons/staj_cursor.hpp                         \
    %reldir%/jsoncons/include/jsoncons/encode_json.hpp                         \
    %reldir%/jsoncons/include/jsoncons/traits_extension.hpp                    \
    %reldir%/jsoncons/include/jsoncons/json_reader.hpp                         \
    %reldir%/jsoncons/include/jsoncons/json_content_handler.hpp                \
    %reldir%/jsoncons/include/jsoncons/basic_json.hpp                          \
    %reldir%/jsoncons/include/jsoncons/allocator_holder.hpp                    \
    %reldir%/jsoncons/include/jsoncons/encode_traits.hpp                       \
    %reldir%/jsoncons/include/jsoncons/json_cursor.hpp                         \
    %reldir%/jsoncons/include/jsoncons/json_traits_macros.hpp                  \
    %reldir%/jsoncons/include/jsoncons/json_traits_macros_deprecated.hpp       \
    %reldir%/jsoncons/include/jsoncons/json_type_traits.hpp                    \
    %reldir%/jsoncons/include/jsoncons/json_parser.hpp                         \
    %reldir%/jsoncons/include/jsoncons/json.hpp                                \
    %reldir%/jsoncons/include/jsoncons/ser_context.hpp                         \
    %reldir%/jsoncons/include/jsoncons/detail/string_wrapper.hpp               \
    %reldir%/jsoncons/include/jsoncons/detail/parse_number.hpp                 \
    %reldir%/jsoncons/include/jsoncons/detail/grisu3.hpp                       \
    %reldir%/jsoncons/include/jsoncons/detail/endian.hpp                       \
    %reldir%/jsoncons/include/jsoncons/detail/optional.hpp                     \
    %reldir%/jsoncons/include/jsoncons/detail/span.hpp                         \
    %reldir%/jsoncons/include/jsoncons/detail/write_number.hpp                 \
    %reldir%/jsoncons/include/jsoncons/detail/string_view.hpp                  \
    %reldir%/jsoncons/include/jsoncons/value_converter.hpp                     \
    %reldir%/jsoncons/include/jsoncons/typed_array_view.hpp                    \
    %reldir%/jsoncons/include/jsoncons/json_array.hpp                          \
    %reldir%/jsoncons/include/jsoncons/json_decoder.hpp                        \
    %reldir%/jsoncons/include/jsoncons/decode_traits.hpp                       \
    %reldir%/jsoncons/include/jsoncons/json_filter.hpp                         \
    %reldir%/jsoncons/include/jsoncons_ext/csv/csv.hpp                         \
    %reldir%/jsoncons/include/jsoncons_ext/csv/csv_cursor.hpp                  \
    %reldir%/jsoncons/include/jsoncons_ext/csv/csv_encoder.hpp                 \
    %reldir%/jsoncons/include/jsoncons_ext/csv/csv_error.hpp                   \
    %reldir%/jsoncons/include/jsoncons_ext/csv/csv_options.hpp                 \
    %reldir%/jsoncons/include/jsoncons_ext/csv/csv_parser.hpp                  \
    %reldir%/jsoncons/include/jsoncons_ext/csv/csv_reader.hpp                  \
    %reldir%/jsoncons/include/jsoncons_ext/csv/csv_serializer.hpp              \
    %reldir%/jsoncons/include/jsoncons_ext/csv/decode_csv.hpp                  \
    %reldir%/jsoncons/include/jsoncons_ext/csv/encode_csv.hpp                  \
    ${NOTHING}

# inotify-cpp
noinst_HEADERS +=                                                              \
    %reldir%/inotify-cpp/src/include/inotify-cpp/Event.h                       \
    %reldir%/inotify-cpp/src/include/inotify-cpp/FileSystemEvent.h             \
    %reldir%/inotify-cpp/src/include/inotify-cpp/Inotify.h                     \
    %reldir%/inotify-cpp/src/include/inotify-cpp/Notification.h                \
    %reldir%/inotify-cpp/src/include/inotify-cpp/NotifierBuilder.h             \
    ${NOTHING}
EXTRA_DIST +=                                                                  \
    %reldir%/inotify-cpp/build/src/libinotify-cpp.so                           \
    %reldir%/inotify-cpp/build/src/libinotify-cpp.so.0.2.0                     \
    ${NOTHING}

# fmt
noinst_HEADERS +=                                                              \
    %reldir%/fmt/include/fmt/std.h                                             \
    %reldir%/fmt/include/fmt/args.h                                            \
    %reldir%/fmt/include/fmt/printf.h                                          \
    %reldir%/fmt/include/fmt/core.h                                            \
    %reldir%/fmt/include/fmt/xchar.h                                           \
    %reldir%/fmt/include/fmt/format.h                                          \
    %reldir%/fmt/include/fmt/color.h                                           \
    %reldir%/fmt/include/fmt/format-inl.h                                      \
    %reldir%/fmt/include/fmt/chrono.h                                          \
    %reldir%/fmt/include/fmt/ostream.h                                         \
    %reldir%/fmt/include/fmt/os.h                                              \
    %reldir%/fmt/include/fmt/ranges.h                                          \
    %reldir%/fmt/include/fmt/compile.h                                         \
    ${NOTHING}

# tclap
noinst_HEADERS +=                                                              \
    %reldir%/tclap/include/tclap/Arg.h                                         \
    %reldir%/tclap/include/tclap/ArgException.h                                \
    %reldir%/tclap/include/tclap/ArgTraits.h                                   \
    %reldir%/tclap/include/tclap/CmdLine.h                                     \
    %reldir%/tclap/include/tclap/CmdLineInterface.h                            \
    %reldir%/tclap/include/tclap/CmdLineOutput.h                               \
    %reldir%/tclap/include/tclap/Constraint.h                                  \
    %reldir%/tclap/include/tclap/DocBookOutput.h                               \
    %reldir%/tclap/include/tclap/HelpVisitor.h                                 \
    %reldir%/tclap/include/tclap/IgnoreRestVisitor.h                           \
    %reldir%/tclap/include/tclap/MultiArg.h                                    \
    %reldir%/tclap/include/tclap/MultiSwitchArg.h                              \
    %reldir%/tclap/include/tclap/OptionalUnlabeledTracker.h                    \
    %reldir%/tclap/include/tclap/StandardTraits.h                              \
    %reldir%/tclap/include/tclap/StdOutput.h                                   \
    %reldir%/tclap/include/tclap/SwitchArg.h                                   \
    %reldir%/tclap/include/tclap/UnlabeledMultiArg.h                           \
    %reldir%/tclap/include/tclap/UnlabeledValueArg.h                           \
    %reldir%/tclap/include/tclap/ValueArg.h                                    \
    %reldir%/tclap/include/tclap/ValuesConstraint.h                            \
    %reldir%/tclap/include/tclap/VersionVisitor.h                              \
    %reldir%/tclap/include/tclap/Visitor.h                                     \
    %reldir%/tclap/include/tclap/XorHandler.h                                  \
    %reldir%/tclap/include/tclap/ZshCompletionOutput.h                         \
    %reldir%/tclap/include/tclap/sstream.h                                     \
    ${NOTHING}
