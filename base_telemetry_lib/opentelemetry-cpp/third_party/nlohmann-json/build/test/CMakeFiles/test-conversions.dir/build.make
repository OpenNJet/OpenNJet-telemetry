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
include test/CMakeFiles/test-conversions.dir/depend.make

# Include the progress variables for this target.
include test/CMakeFiles/test-conversions.dir/progress.make

# Include the compile flags for this target's objects.
include test/CMakeFiles/test-conversions.dir/flags.make

test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o: test/CMakeFiles/test-conversions.dir/flags.make
test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o: ../test/src/unit-conversions.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o"
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o -c /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/src/unit-conversions.cpp

test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.i"
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/src/unit-conversions.cpp > CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.i

test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.s"
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && /usr/bin/c++ $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test/src/unit-conversions.cpp -o CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.s

test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o.requires:

.PHONY : test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o.requires

test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o.provides: test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o.requires
	$(MAKE) -f test/CMakeFiles/test-conversions.dir/build.make test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o.provides.build
.PHONY : test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o.provides

test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o.provides.build: test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o


# Object files for target test-conversions
test__conversions_OBJECTS = \
"CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o"

# External object files for target test-conversions
test__conversions_EXTERNAL_OBJECTS = \
"/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/CMakeFiles/doctest_main.dir/src/unit.cpp.o"

test/test-conversions: test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o
test/test-conversions: test/CMakeFiles/doctest_main.dir/src/unit.cpp.o
test/test-conversions: test/CMakeFiles/test-conversions.dir/build.make
test/test-conversions: test/CMakeFiles/test-conversions.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX executable test-conversions"
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test-conversions.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
test/CMakeFiles/test-conversions.dir/build: test/test-conversions

.PHONY : test/CMakeFiles/test-conversions.dir/build

test/CMakeFiles/test-conversions.dir/requires: test/CMakeFiles/test-conversions.dir/src/unit-conversions.cpp.o.requires

.PHONY : test/CMakeFiles/test-conversions.dir/requires

test/CMakeFiles/test-conversions.dir/clean:
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test && $(CMAKE_COMMAND) -P CMakeFiles/test-conversions.dir/cmake_clean.cmake
.PHONY : test/CMakeFiles/test-conversions.dir/clean

test/CMakeFiles/test-conversions.dir/depend:
	cd /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /telemetry/opentelemetry-cpp/third_party/nlohmann-json /telemetry/opentelemetry-cpp/third_party/nlohmann-json/test /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test /telemetry/opentelemetry-cpp/third_party/nlohmann-json/build/test/CMakeFiles/test-conversions.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : test/CMakeFiles/test-conversions.dir/depend
