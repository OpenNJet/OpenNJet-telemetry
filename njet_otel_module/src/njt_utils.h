#pragma once

#include <opentelemetry/nostd/string_view.h>

extern "C" {
#include <njt_string.h>
}

inline opentelemetry::nostd::string_view FromNjtString(njt_str_t str) {
  return {(const char*)str.data, str.len};
}
inline njt_str_t ToNjtString(opentelemetry::nostd::string_view str) {
  return {str.size(), (u_char*)str.data()};
}
