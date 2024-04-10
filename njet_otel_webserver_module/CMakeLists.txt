cmake_minimum_required(VERSION 3.12)

project(opentelemetry-njt)

#find_package(opentelemetry-cpp REQUIRED)
#find_package(Threads REQUIRED)
#find_package(Protobuf REQUIRED)
#find_package(gRPC REQUIRED)


#set(gRPC_FOUND true)
#set(gRPC_INCLUDE_DIR /usr/include)
#set(gRPC_LIBRARY /usr/lib64)

#find_package(CURL REQUIRED)

#include(${CMAKE_CURRENT_SOURCE_DIR}/njt.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/njt.cmake)

add_library(njt_otel_webserver_module SHARED
  src/njt_http_opentelemetry_log.c
  src/njt_http_opentelemetry_module.c
  src/njt_http_opentelemetry_module_modules.c
)


add_library(njt_agent_dyn_otel_webserver_module SHARED
  src/njet-dyn-otel-webserver-module/src/njt_agent_dyn_otel_webserver.c
  src/njet-dyn-otel-webserver-module/src/njt_dyn_telemetry_ws_parser.c
  src/njet-dyn-otel-webserver-module/src/njt_agent_dyn_otel_webserver_module_modules.c
)


target_compile_options(njt_otel_webserver_module
  PRIVATE -g -Wall -Wextra -L../lib/
)


target_compile_options(njt_agent_dyn_otel_webserver_module
  PRIVATE -g -Wall -Wextra -L../lib/
)


install(TARGETS njt_otel_webserver_module DESTINATION ".")
install(TARGETS njt_agent_dyn_otel_webserver_module DESTINATION ".")

# Omit the lib prefix to be more in line with njt module naming
set_target_properties(njt_otel_webserver_module PROPERTIES
  PREFIX ""
)

set_target_properties(njt_agent_dyn_otel_webserver_module PROPERTIES
  PREFIX ""
)
#add_dependencies(njt_otel_webserver_module project_njt)


add_definitions(-DNJT_VAR_INIT_REF_COUNT=1)
add_definitions(-DNJT_HTTP_DYNAMIC_LOC)
add_definitions(-DCMAKE_BUILD_TYPE=Debug)


set(NJET_DYN_OTEL_WEBSERVER_INCLUDE_DIRS
  ${CMAKE_CURRENT_SOURCE_DIR}/src/njet-dyn-otel-webserver-module/src/
  ${CMAKE_CURRENT_SOURCE_DIR}/src/
)


target_include_directories(njt_otel_webserver_module
  PRIVATE
  ${NJET_INCLUDE_DIRS}
  ${OPENTELEMETRY_CPP_INCLUDE_DIRS}
  include/core/api/
)


target_include_directories(njt_agent_dyn_otel_webserver_module
  PRIVATE
  ${NJET_INCLUDE_DIRS}
  ${OPENTELEMETRY_CPP_INCLUDE_DIRS}
  ${NJET_DYN_OTEL_WEBSERVER_INCLUDE_DIRS}
  include/core/api/
)


add_library(opentelemetry_webserver_sdk SHARED IMPORTED )
set_target_properties(opentelemetry_webserver_sdk PROPERTIES   IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/../lib/libopentelemetry_webserver_sdk.so")


link_directories(
   ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

set(CMAKE_INSTALL_RPATH "./lib")

target_link_libraries(njt_otel_webserver_module
  #PRIVATE
  opentelemetry_webserver_sdk
)

set_property(TARGET njt_otel_webserver_module PROPERTY C_STANDARD 99)
