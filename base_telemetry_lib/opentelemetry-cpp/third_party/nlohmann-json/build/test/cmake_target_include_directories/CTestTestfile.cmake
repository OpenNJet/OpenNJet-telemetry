# CMake generated Testfile for 
# Source directory: /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/cmake_target_include_directories
# Build directory: /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/cmake_target_include_directories
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(cmake_target_include_directories_configure "/usr/bin/cmake" "-G" "Unix Makefiles" "-DCMAKE_CXX_COMPILER=/usr/bin/c++" "-Dnlohmann_json_source=/telemetry/opentelemetry-cpp/third_party/nlohmann-json" "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/cmake_target_include_directories/project")
set_tests_properties(cmake_target_include_directories_configure PROPERTIES  FIXTURES_SETUP "cmake_target_include_directories")
add_test(cmake_target_include_directories_build "/usr/bin/cmake" "--build" ".")
set_tests_properties(cmake_target_include_directories_build PROPERTIES  FIXTURES_REQUIRED "cmake_target_include_directories")
