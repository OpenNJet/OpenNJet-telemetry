# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /telemetry/opentelemetry-cpp/third_party/nlohmann-json

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build

# Include any dependencies generated for this target.
include test/CMakeFiles/test-hash.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test-hash.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test-hash.dir/flags.make

test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o: test/CMakeFiles/test-hash.dir/flags.make
test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o: ../test/src/unit-hash.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o"
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test-hash.dir/src/unit-hash.cpp.o -c /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/src/unit-hash.cpp

test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test-hash.dir/src/unit-hash.cpp.i"
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/src/unit-hash.cpp > CMakeFiles/test-hash.dir/src/unit-hash.cpp.i

test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test-hash.dir/src/unit-hash.cpp.s"
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/src/unit-hash.cpp -o CMakeFiles/test-hash.dir/src/unit-hash.cpp.s

test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o.requires:

.PHONY : test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o.requires

test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o.provides: test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o.requires
	$(MAKE) -f test/CMakeFiles/test-hash.dir/build.make test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o.provides.build
.PHONY : test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o.provides

test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o.provides.build: test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o


# Object files for target test-hash
test__hash_OBJECTS = \
"CMakeFiles/test-hash.dir/src/unit-hash.cpp.o"

# External object files for target test-hash
test__hash_EXTERNAL_OBJECTS = \
"/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/CMakeFiles/doctest_main.dir/src/unit.cpp.o"

test/test-hash: test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o
test/test-hash: test/CMakeFiles/doctest_main.dir/src/unit.cpp.o
test/test-hash: test/CMakeFiles/test-hash.dir/build.make
test/test-hash: test/CMakeFiles/test-hash.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable test-hash"
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test-hash.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test-hash.dir/build: test/test-hash

.PHONY : test/CMakeFiles/test-hash.dir/build

test/CMakeFiles/test-hash.dir/requires: test/CMakeFiles/test-hash.dir/src/unit-hash.cpp.o.requires

.PHONY : test/CMakeFiles/test-hash.dir/requires

test/CMakeFiles/test-hash.dir/clean:
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && $(CMAKE_COMMAND) -P CMakeFiles/test-hash.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test-hash.dir/clean

test/CMakeFiles/test-hash.dir/depend:
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /telemetry/opentelemetry-cpp/third_party/nlohmann-json /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/CMakeFiles/test-hash.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test-hash.dir/depend

