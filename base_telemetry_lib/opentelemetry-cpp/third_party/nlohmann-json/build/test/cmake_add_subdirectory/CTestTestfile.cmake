# CMake generated Testfile for 
# Source directory: /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/cmake_add_subdirectory
# Build directory: /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/cmake_add_subdirectory
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(cmake_add_subdirectory_configure "/usr/bin/cmake" "-G" "Unix Makefiles" "-DCMAKE_CXX_COMPILER=/usr/bin/c++" "-Dnlohmann_json_source=/telemetry/opentelemetry-cpp/third_party/nlohmann-json" "/telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/cmake_add_subdirectory/project")
set_tests_properties(cmake_add_subdirectory_configure PROPERTIES  FIXTURES_SETUP "cmake_add_subdirectory")
add_test(cmake_add_subdirectory_build "/usr/bin/cmake" "--build" ".")
set_tests_properties(cmake_add_subdirectory_build PROPERTIES  FIXTURES_REQUIRED "cmake_add_subdirectory")
