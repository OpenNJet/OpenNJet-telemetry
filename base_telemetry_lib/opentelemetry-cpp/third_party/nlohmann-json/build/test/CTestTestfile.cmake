# CMake generated Testfile for 
# Source directory: /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test
# Build directory: /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(download_test_data "/usr/bin/cmake" "--build" "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build" "--target" "download_test_data")
set_tests_properties(download_test_data PROPERTIES  FIXTURES_SETUP "TEST_DATA")
add_test(test-algorithms "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-algorithms" "--no-skip")
set_tests_properties(test-algorithms PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-allocator "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-allocator" "--no-skip")
set_tests_properties(test-allocator PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-alt-string "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-alt-string" "--no-skip")
set_tests_properties(test-alt-string PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-assert_macro "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-assert_macro" "--no-skip")
set_tests_properties(test-assert_macro PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-bson "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-bson" "--no-skip")
set_tests_properties(test-bson PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-capacity "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-capacity" "--no-skip")
set_tests_properties(test-capacity PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-cbor "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-cbor" "--no-skip")
set_tests_properties(test-cbor PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-class_const_iterator "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-class_const_iterator" "--no-skip")
set_tests_properties(test-class_const_iterator PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-class_iterator "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-class_iterator" "--no-skip")
set_tests_properties(test-class_iterator PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-class_lexer "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-class_lexer" "--no-skip")
set_tests_properties(test-class_lexer PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-class_parser "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-class_parser" "--no-skip")
set_tests_properties(test-class_parser PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-comparison "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-comparison" "--no-skip")
set_tests_properties(test-comparison PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-concepts "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-concepts" "--no-skip")
set_tests_properties(test-concepts PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-constructor1 "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-constructor1" "--no-skip")
set_tests_properties(test-constructor1 PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-constructor2 "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-constructor2" "--no-skip")
set_tests_properties(test-constructor2 PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-convenience "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-convenience" "--no-skip")
set_tests_properties(test-convenience PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-conversions "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-conversions" "--no-skip")
set_tests_properties(test-conversions PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-deserialization "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-deserialization" "--no-skip")
set_tests_properties(test-deserialization PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-element_access1 "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-element_access1" "--no-skip")
set_tests_properties(test-element_access1 PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-element_access2 "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-element_access2" "--no-skip")
set_tests_properties(test-element_access2 PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-hash "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-hash" "--no-skip")
set_tests_properties(test-hash PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-inspection "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-inspection" "--no-skip")
set_tests_properties(test-inspection PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-items "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-items" "--no-skip")
set_tests_properties(test-items PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-iterators1 "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-iterators1" "--no-skip")
set_tests_properties(test-iterators1 PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-iterators2 "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-iterators2" "--no-skip")
set_tests_properties(test-iterators2 PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-json_patch "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-json_patch" "--no-skip")
set_tests_properties(test-json_patch PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-json_pointer "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-json_pointer" "--no-skip")
set_tests_properties(test-json_pointer PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-large_json "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-large_json" "--no-skip")
set_tests_properties(test-large_json PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-merge_patch "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-merge_patch" "--no-skip")
set_tests_properties(test-merge_patch PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-meta "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-meta" "--no-skip")
set_tests_properties(test-meta PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-modifiers "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-modifiers" "--no-skip")
set_tests_properties(test-modifiers PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-msgpack "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-msgpack" "--no-skip")
set_tests_properties(test-msgpack PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-noexcept "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-noexcept" "--no-skip")
set_tests_properties(test-noexcept PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-ordered_json "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-ordered_json" "--no-skip")
set_tests_properties(test-ordered_json PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-ordered_map "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-ordered_map" "--no-skip")
set_tests_properties(test-ordered_map PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-pointer_access "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-pointer_access" "--no-skip")
set_tests_properties(test-pointer_access PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-readme "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-readme" "--no-skip")
set_tests_properties(test-readme PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-reference_access "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-reference_access" "--no-skip")
set_tests_properties(test-reference_access PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-regression1 "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-regression1" "--no-skip")
set_tests_properties(test-regression1 PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-regression2 "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-regression2" "--no-skip")
set_tests_properties(test-regression2 PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-serialization "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-serialization" "--no-skip")
set_tests_properties(test-serialization PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-testsuites "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-testsuites" "--no-skip")
set_tests_properties(test-testsuites PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-to_chars "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-to_chars" "--no-skip")
set_tests_properties(test-to_chars PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-ubjson "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-ubjson" "--no-skip")
set_tests_properties(test-ubjson PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-udt "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-udt" "--no-skip")
set_tests_properties(test-udt PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-udt_macro "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-udt_macro" "--no-skip")
set_tests_properties(test-udt_macro PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-unicode "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-unicode" "--no-skip")
set_tests_properties(test-unicode PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-user_defined_input "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-user_defined_input" "--no-skip")
set_tests_properties(test-user_defined_input PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
add_test(test-wstring "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/test-wstring" "--no-skip")
set_tests_properties(test-wstring PROPERTIES  FIXTURES_REQUIRED "TEST_DATA" LABELS "all" WORKING_DIRECTORY "/telemetry/opentelemetry-cpp/third_party/nlohmann-json")
subdirs("cmake_import")
subdirs("cmake_import_minver")
subdirs("cmake_add_subdirectory")
subdirs("cmake_fetch_content")
subdirs("cmake_target_include_directories")
