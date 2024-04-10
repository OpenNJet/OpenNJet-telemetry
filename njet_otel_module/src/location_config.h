#pragma once

#include "trace_context.h"
#include "script.h"

extern "C" {
extern njt_module_t njt_otel_module;
}

struct OtelNjtLocationConf {
  njt_flag_t enabled = NJT_CONF_UNSET;
  njt_flag_t trustIncomingSpans = NJT_CONF_UNSET;
  njt_flag_t captureHeaders = NJT_CONF_UNSET;
#if (NJT_PCRE)
  njt_regex_t *sensitiveHeaderNames = (njt_regex_t*)NJT_CONF_UNSET_PTR;
  njt_regex_t *sensitiveHeaderValues = (njt_regex_t*)NJT_CONF_UNSET_PTR;
  njt_regex_t *ignore_paths = (njt_regex_t*)NJT_CONF_UNSET_PTR;
#endif
  TracePropagationType propagationType = TracePropagationUnset;
  NjtCompiledScript operationNameScript;
  njt_array_t* customAttributes = nullptr;
};

inline OtelNjtLocationConf* GetOtelLocationConf(njt_http_request_t* req) {
  return (OtelNjtLocationConf*)njt_http_get_module_loc_conf(req, njt_otel_module);
}
