cmake_minimum_required(VERSION 3.12)

project(opentelemetry-njt)

include(${CMAKE_CURRENT_SOURCE_DIR}/njt.cmake)

set(OPENTELEMETRY_CPP_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/opentelemetry/1.2.0/include/)
set(OPENTELEMETRY_CPP_LIBRARIES opentelemetry_common  opentelemetry_exporter_ostream_span opentelemetry_exporter_otlp_grpc opentelemetry_otlp_recordable opentelemetry_resources opentelemetry_trace)


add_library(njt_otel_module SHARED
  src/njt_config.cpp
  src/toml.c
  src/agent_config.cpp
  src/trace_context.cpp
  src/njt_otel_module.cpp
  src/njt_otel_module_modules.c
  src/propagate.cpp
  src/script.cpp
)

add_library(njt_agent_dyn_otel_module SHARED
  src/njet-dyn-otel-module/src/njt_agent_dyn_telemetry.cpp
  src/njet-dyn-otel-module/src/njt_dyn_telemetry_parser.c
  src/njet-dyn-otel-module/src/njt_agent_dyn_telemetry_module_modules.c
)

set(CMAKE_CXX_STANDARD 11)

target_compile_options(njt_otel_module
  PRIVATE -Wall -Wextra -std=c++11 -L${CMAKE_CURRENT_SOURCE_DIR}/../lib/
)

target_compile_options(njt_agent_dyn_otel_module
  PRIVATE -Wall -Wextra -std=c++11
)

install(TARGETS njt_otel_module DESTINATION ".")
install(TARGETS njt_agent_dyn_otel_module DESTINATION ".")

# Omit the lib prefix to be more in line with njt module naming
set_target_properties(njt_otel_module PROPERTIES
  PREFIX ""
)

set_target_properties(njt_agent_dyn_otel_module PROPERTIES
  PREFIX ""
)


add_definitions(-DNJT_VAR_INIT_REF_COUNT=1)
add_definitions(-DNJT_HTTP_DYNAMIC_LOC)
add_definitions(-DCMAKE_BUILD_TYPE=Debug)


set(NJET_DYN_TELEMETRY_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/njet-dyn-otel-module/src/
  ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_include_directories(njt_otel_module
  PRIVATE
  ${NJET_INCLUDE_DIRS}
  ${OPENTELEMETRY_CPP_INCLUDE_DIRS}
)

target_include_directories(njt_agent_dyn_otel_module
  PRIVATE
  ${NJET_INCLUDE_DIRS}
  ${NJET_DYN_TELEMETRY_INCLUDE_DIRS}
  ${OPENTELEMETRY_CPP_INCLUDE_DIRS}
)

target_link_libraries(njt_otel_module
  PRIVATE
  ${OPENTELEMETRY_CPP_LIBRARIES}
)

set_property(TARGET njt_otel_module PROPERTY C_STANDARD 99)
set_property(TARGET njt_agent_dyn_otel_module PROPERTY C_STANDARD 99)
