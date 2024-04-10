// clang-format off
// otlp_grpc_exporter header has to be included before any other API header to 
// avoid conflict between Abseil library and OpenTelemetry C++ absl::variant.
// https://github.com/open-telemetry/opentelemetry-cpp/tree/main/examples/otlp#additional-notes-regarding-abseil-library
#include <opentelemetry/exporters/otlp/otlp_grpc_exporter.h>
// clang-format on

#include <opentelemetry/sdk/trace/processor.h>
#include <opentelemetry/trace/span.h>
#include <algorithm>
#include <unordered_map>
#include <vector>

extern "C" {
#include <njt_config.h>
#include <njt_core.h>
#include <njt_http.h>

extern njt_module_t njt_otel_module;
}

#include "agent_config.h"
#include "location_config.h"
#include "njt_config.h"
#include "njt_utils.h"
#include "propagate.h"
#include <opentelemetry/context/context.h>
#include <opentelemetry/nostd/shared_ptr.h>
#include <opentelemetry/sdk/trace/batch_span_processor.h>
#include <opentelemetry/sdk/trace/id_generator.h>
#include <opentelemetry/sdk/trace/samplers/always_off.h>
#include <opentelemetry/sdk/trace/samplers/always_on.h>
#include <opentelemetry/sdk/trace/samplers/parent.h>
#include <opentelemetry/sdk/trace/samplers/trace_id_ratio.h>
#include <opentelemetry/sdk/trace/simple_processor.h>
#include <opentelemetry/sdk/trace/tracer_provider.h>
#include <opentelemetry/trace/provider.h>

namespace trace = opentelemetry::trace;
namespace nostd = opentelemetry::nostd;
namespace sdktrace = opentelemetry::sdk::trace;
namespace otlp = opentelemetry::exporter::otlp;

constexpr char kOtelCtxVarPrefix[] = "opentelemetry_context_";

const ScriptAttributeDeclaration kDefaultScriptAttributes[] = {
  {"http.scheme", "$scheme"},
  {"net.host.port", "$server_port", ScriptAttributeInt},
  {"net.peer.ip", "$remote_addr"},
  {"net.peer.port", "$remote_port", ScriptAttributeInt},
};

struct OtelMainConf {
  njt_array_t* scriptAttributes;
  OtelNjtAgentConfig agentConfig;
};

nostd::shared_ptr<trace::Tracer> GetTracer() {
  return trace::Provider::GetTracerProvider()->GetTracer("njt");
}

nostd::string_view NjtHttpFlavor(njt_http_request_t* req) {
  switch (req->http_version) {
    case NJT_HTTP_VERSION_11:
      return "1.1";
    case NJT_HTTP_VERSION_20:
      return "2.0";
    case NJT_HTTP_VERSION_10:
      return "1.0";
    default:
      return "";
  }
}

static void NjtNormalizeAndCopyString(u_char* dst, njt_str_t str) {
  for (njt_uint_t i = 0; i < str.len; ++i) {
    u_char ch = str.data[i];
    if (ch >= 'A' && ch <= 'Z') {
      ch += 0x20;
    } else if (ch == '-') {
      ch = '_';
    }

    dst[i] = ch;
  }
}

static void OtelCaptureHeaders(nostd::shared_ptr<opentelemetry::trace::Span> span, njt_str_t keyPrefix, njt_list_t *headers,
#if (NJT_PCRE)
                        njt_regex_t *sensitiveHeaderNames, njt_regex_t *sensitiveHeaderValues,
#endif
                        nostd::span<njt_table_elt_t*> excludedHeaders) {
  for (njt_list_part_t *part = &headers->part; part != nullptr; part = part->next) {
    njt_table_elt_t *header = (njt_table_elt_t*) part->elts;
    for (njt_uint_t i = 0; i < part->nelts; ++i) {
      if (std::find(excludedHeaders.begin(), excludedHeaders.end(), &header[i]) != excludedHeaders.end()) {
        continue;
      }

      u_char key[keyPrefix.len + header[i].key.len]; 
      NjtNormalizeAndCopyString((u_char*)njt_copy(key, keyPrefix.data, keyPrefix.len), header[i].key);

      bool sensitiveHeader = false;
#if (NJT_PCRE)
      if (sensitiveHeaderNames) {
        int ovector[3];
        if (njt_regex_exec(sensitiveHeaderNames, &header[i].key, ovector, 0) >= 0) {
          sensitiveHeader = true;
        }
      }
      if (sensitiveHeaderValues && !sensitiveHeader) {
        int ovector[3];
        if (njt_regex_exec(sensitiveHeaderValues, &header[i].value, ovector, 0) >= 0) {
          sensitiveHeader = true;
        }
      }
#endif

      nostd::string_view value;
      if (sensitiveHeader) {
        value = "[REDACTED]";
      } else {
        value = FromNjtString(header[i].value);
      }

      span->SetAttribute({(const char*)key, keyPrefix.len + header[i].key.len}, nostd::span<const nostd::string_view>(&value, 1));
    }
  }
}

static njt_int_t OtelGetContextVar(njt_http_request_t*, njt_http_variable_value_t*, uintptr_t) {
  // Filled out on context creation.
  return NJT_OK;
}

static njt_int_t
OtelGetTraceContextVar(njt_http_request_t* req, njt_http_variable_value_t* v, uintptr_t data);

static njt_int_t
OtelGetTraceId(njt_http_request_t* req, njt_http_variable_value_t* v, uintptr_t data);

static njt_int_t
OtelGetSpanId(njt_http_request_t* req, njt_http_variable_value_t* v, uintptr_t data);

static njt_http_variable_t otel_njt_variables[] = {
  {
    njt_string("otel_ctx"),
    nullptr,
    OtelGetContextVar,
    0,
    NJT_HTTP_VAR_NOCACHEABLE | NJT_HTTP_VAR_NOHASH,
    0,
    NJT_VAR_INIT_REF_COUNT,
  },
  {
    njt_string(kOtelCtxVarPrefix),
    nullptr,
    OtelGetTraceContextVar,
    0,
    NJT_HTTP_VAR_PREFIX | NJT_HTTP_VAR_NOHASH | NJT_HTTP_VAR_NOCACHEABLE,
    0,
    NJT_VAR_INIT_REF_COUNT,
  },
  {
    njt_string("opentelemetry_trace_id"),
    nullptr,
    OtelGetTraceId,
    0,
    NJT_HTTP_VAR_NOCACHEABLE | NJT_HTTP_VAR_NOHASH,
    0,
    NJT_VAR_INIT_REF_COUNT,
  },
  {
    njt_string("opentelemetry_span_id"),
    nullptr,
    OtelGetSpanId,
    0,
    NJT_HTTP_VAR_NOCACHEABLE | NJT_HTTP_VAR_NOHASH,
    0,
    NJT_VAR_INIT_REF_COUNT,
  },
  njt_http_null_variable,
};

static bool IsOtelEnabled(njt_http_request_t* req) {
  OtelNjtLocationConf* locConf = GetOtelLocationConf(req);
  if(locConf == nullptr){
    return false;
  }

  if (locConf->enabled) {
#if (NJT_PCRE)
    int ovector[3];
    return locConf->ignore_paths == nullptr || njt_regex_exec(locConf->ignore_paths, &req->unparsed_uri, ovector, 0) < 0;
#else
    return true;
#endif
  } else {
    return false;
  }
}

TraceContext* GetTraceContext(njt_http_request_t* req) {
  njt_http_variable_value_t* val = njt_http_get_indexed_variable(req, otel_njt_variables[0].index);

  if (val == nullptr || val->not_found) {
    njt_log_error(NJT_LOG_INFO, req->connection->log, 0, "TraceContext not found");
    return nullptr;
  }

  std::unordered_map<njt_http_request_t*, TraceContext*>* map = (std::unordered_map<njt_http_request_t*, TraceContext*>*)val->data;
  if (map == nullptr){
    njt_log_error(NJT_LOG_INFO, req->connection->log, 0, "TraceContext not found");
    return nullptr;
  }
  auto it = map->find(req);
  if (it != map->end()) {
    return it->second;
  }
  njt_log_error(NJT_LOG_INFO, req->connection->log, 0, "TraceContext not found");
  return nullptr;
}

nostd::string_view WithoutOtelVarPrefix(njt_str_t value) {
  const size_t prefixLength = sizeof(kOtelCtxVarPrefix) - 1;

  if (value.len <= prefixLength) {
    return "";
  }

  return {(const char*)value.data + prefixLength, value.len - prefixLength};
}

static njt_int_t
OtelGetTraceContextVar(njt_http_request_t* req, njt_http_variable_value_t* v, uintptr_t data) {
  if (!IsOtelEnabled(req)) {
    v->valid = 0;
    v->not_found = 1;
    return NJT_OK;
  }

  TraceContext* traceContext = GetTraceContext(req);

  if (traceContext == nullptr || !traceContext->request_span) {
    njt_log_error(
      NJT_LOG_INFO, req->connection->log, 0,
      "Unable to get trace context when expanding tracecontext var");
    return NJT_OK;
  }

  njt_str_t* prefixedKey = (njt_str_t*)data;

  nostd::string_view key = WithoutOtelVarPrefix(*prefixedKey);

  const TraceHeader* header = TraceContextFindTraceHeader(traceContext, key);

  if (header) {
    v->len = header->value.len;
    v->valid = 1;
    v->no_cacheable = 1;
    v->not_found = 0;
    v->data = header->value.data;
  } else {
    v->len = 0;
    v->valid = 0;
    v->not_found = 1;
    v->no_cacheable = 1;
    v->data = nullptr;
  }

  return NJT_OK;
}

static njt_int_t
OtelGetTraceId(njt_http_request_t* req, njt_http_variable_value_t* v, uintptr_t data) {
  if (!IsOtelEnabled(req)) {
    v->valid = 0;
    v->not_found = 1;
    return NJT_OK;
  }

  TraceContext* traceContext = GetTraceContext(req);

  if (traceContext == nullptr || !traceContext->request_span) {
    njt_log_error(
      NJT_LOG_INFO, req->connection->log, 0,
      "Unable to get trace context when getting trace id");
    return NJT_OK;
  }

  trace::SpanContext spanContext = traceContext->request_span->GetContext();

  if (spanContext.IsValid()) {
    constexpr int len = 2 * trace::TraceId::kSize;
    char* data = (char*)njt_palloc(req->pool, len);

    if(!data) {
      njt_log_error(
        NJT_LOG_ERR, req->connection->log, 0,
        "Unable to allocate memory for the trace id");

      v->len = 0;
      v->valid = 0;
      v->no_cacheable = 1;
      v->not_found = 0;
      v->data = nullptr;

      return NJT_OK;
    }

    spanContext.trace_id().ToLowerBase16(nostd::span<char, len>{data, len});

    v->len = len;
    v->valid = 1;
    v->no_cacheable = 1;
    v->not_found = 0;
    v->data = (u_char*)data;
  } else {
    v->len = 0;
    v->valid = 0;
    v->no_cacheable = 1;
    v->not_found = 1;
    v->data = nullptr;
  }

  return NJT_OK;
}

static njt_int_t
OtelGetSpanId(njt_http_request_t* req, njt_http_variable_value_t* v, uintptr_t data) {
  if (!IsOtelEnabled(req)) {
    v->valid = 0;
    v->not_found = 1;
    return NJT_OK;
  }

  TraceContext* traceContext = GetTraceContext(req);

  if (traceContext == nullptr || !traceContext->request_span) {
    njt_log_error(
      NJT_LOG_INFO, req->connection->log, 0,
      "Unable to get trace context when getting span id");
    return NJT_OK;
  }

  trace::SpanContext spanContext = traceContext->request_span->GetContext();

  if (spanContext.IsValid()) {
    constexpr int len = 2 * trace::SpanId::kSize;
    char* data = (char*)njt_palloc(req->pool, len);

    if(!data) {
      njt_log_error(
        NJT_LOG_ERR, req->connection->log, 0,
        "Unable to allocate memory for the span id");

      v->len = 0;
      v->valid = 0;
      v->no_cacheable = 1;
      v->not_found = 0;
      v->data = nullptr;

      return NJT_OK;
    }

    spanContext.span_id().ToLowerBase16(nostd::span<char, len>{data, len});

    v->len = len;
    v->valid = 1;
    v->no_cacheable = 1;
    v->not_found = 0;
    v->data = (u_char*)data;
  } else {
    v->len = 0;
    v->valid = 0;
    v->no_cacheable = 1;
    v->not_found = 1;
    v->data = nullptr;
  }

  return NJT_OK;
}

void TraceContextCleanup(void* data) {
  TraceContext* context = (TraceContext*)data;
  context->~TraceContext();
}

void RequestContextMapCleanup(void* data) {
  std::unordered_map<njt_http_request_t*, TraceContext*>* map = (std::unordered_map<njt_http_request_t*, TraceContext*>*)data;
  map->~unordered_map();
}

nostd::string_view GetOperationName(njt_http_request_t* req) {
  OtelNjtLocationConf* locationConf = GetOtelLocationConf(req);

  njt_str_t opName = njt_null_string;
  if(locationConf == nullptr){
    return FromNjtString(opName);
  }
  if (locationConf->operationNameScript.Run(req, &opName)) {
    return FromNjtString(opName);
  }

  njt_http_core_loc_conf_t* httpCoreLocationConf =
    (njt_http_core_loc_conf_t*)njt_http_get_module_loc_conf(req, njt_http_core_module);

  if (httpCoreLocationConf) {
    return FromNjtString(httpCoreLocationConf->name);
  }

  return FromNjtString(opName);
}

njt_http_core_main_conf_t* NjtHttpModuleMainConf(njt_http_request_t* req) {
  return (njt_http_core_main_conf_t*)njt_http_get_module_main_conf(req, njt_http_core_module);
}

OtelMainConf* GetOtelMainConf(njt_http_request_t* req) {
  return (OtelMainConf*)njt_http_get_module_main_conf(req, njt_otel_module);
}

nostd::string_view GetNjtServerName(const njt_http_request_t* req) {
  njt_http_core_srv_conf_t* cscf =
    (njt_http_core_srv_conf_t*)njt_http_get_module_srv_conf(req, njt_http_core_module);

  njt_str_t opName = njt_null_string;
  if(cscf == nullptr){
    return FromNjtString(opName);
  }

  return FromNjtString(cscf->server_name);
}

TraceContext* CreateTraceContext(njt_http_request_t* req, njt_http_variable_value_t* val) {
  njt_pool_cleanup_t* cleanup = njt_pool_cleanup_add(req->pool, sizeof(TraceContext));
  TraceContext* context = (TraceContext*)cleanup->data;
  new (context) TraceContext(req);
  cleanup->handler = TraceContextCleanup;

  std::unordered_map<njt_http_request_t*, TraceContext*>* map;
  if (req->parent && val->data) {
    // Subrequests will already have the map created so just retrieve it
    map = (std::unordered_map<njt_http_request_t*, TraceContext*>*)val->data;
  } else {
    njt_pool_cleanup_t* cleanup = njt_pool_cleanup_add(req->pool, sizeof(std::unordered_map<njt_http_request_t*, TraceContext*>));
    map = (std::unordered_map<njt_http_request_t*, TraceContext*>*)cleanup->data;
    new (map) std::unordered_map<njt_http_request_t*, TraceContext*>();
    cleanup->handler = RequestContextMapCleanup;
    val->data = (unsigned char*)cleanup->data;
    val->len = sizeof(std::unordered_map<njt_http_request_t*, TraceContext*>);
  }
  map->insert({req, context});
  return context;
}

njt_int_t StartNjtSpan(njt_http_request_t* req) {
  if (!IsOtelEnabled(req)) {
    return NJT_DECLINED;
  }

  // Internal requests must be called from another location in njt, that should already have a trace. Without this check, a call would generate an extra (unrelated) span without much information
  if (req->internal) {
    return NJT_DECLINED;
  }

  njt_http_variable_value_t* val = njt_http_get_indexed_variable(req, otel_njt_variables[0].index);

  if (!val) {
    njt_log_error(NJT_LOG_ERR, req->connection->log, 0, "Unable to find OpenTelemetry context");
    return NJT_DECLINED;
  }

  TraceContext* context = CreateTraceContext(req, val);

  OtelCarrier carrier{req, context};
  opentelemetry::context::Context incomingContext;

  OtelNjtLocationConf* locConf = GetOtelLocationConf(req);
  if(locConf == nullptr){
    njt_log_error(NJT_LOG_ERR, req->connection->log, 0, "Unable to find OpenTelemetry config");
    return NJT_DECLINED;
  }

  if (locConf->trustIncomingSpans) {
    incomingContext = ExtractContext(&carrier);
  }

  trace::StartSpanOptions startOpts;
  startOpts.kind = trace::SpanKind::kServer;
  startOpts.parent = GetCurrentSpan(incomingContext);

  context->request_span = GetTracer()->StartSpan(
    GetOperationName(req),
    {
      {"http.method", FromNjtString(req->method_name)},
      {"http.flavor", NjtHttpFlavor(req)},
      {"http.target", FromNjtString(req->unparsed_uri)},
    },
    startOpts);

  nostd::string_view serverName = GetNjtServerName(req);
  if (!serverName.empty()) {
    context->request_span->SetAttribute("http.server_name", serverName);
  }

  if (req->headers_in.host) {
    context->request_span->SetAttribute("http.host", FromNjtString(req->headers_in.host->value));
  }

  if (req->headers_in.user_agent) {
    context->request_span->SetAttribute("http.user_agent", FromNjtString(req->headers_in.user_agent->value));
  }

  if (locConf->captureHeaders) {
    njt_table_elt_t* excludedHeaders[] = {req->headers_in.host, req->headers_in.user_agent};
    OtelCaptureHeaders(context->request_span, njt_string("http.request.header."), &req->headers_in.headers,
#if (NJT_PCRE)
                       locConf->sensitiveHeaderNames, locConf->sensitiveHeaderValues,
#endif
                       {excludedHeaders, 2});
  }

  auto outgoingContext = incomingContext.SetValue(trace::kSpanKey, context->request_span);

  InjectContext(&carrier, outgoingContext);

  return NJT_DECLINED;
}

void AddScriptAttributes(
  trace::Span* span, const njt_array_t* attributes, njt_http_request_t* req) {

  if (!attributes) {
    return;
  }

  CompiledScriptAttribute* elements = (CompiledScriptAttribute*)attributes->elts;
  for (njt_uint_t i = 0; i < attributes->nelts; i++) {
    CompiledScriptAttribute* attribute = &elements[i];
    njt_str_t key = njt_null_string;
    njt_str_t value = njt_null_string;

    if (attribute->key.Run(req, &key) && attribute->value.Run(req, &value)) {
      switch(attribute->type) {
        case ScriptAttributeInt:
          span->SetAttribute(FromNjtString(key), njt_atoi(value.data, value.len));
          break;
        default:
          span->SetAttribute(FromNjtString(key), FromNjtString(value));
      }
    }
  }
}

njt_int_t FinishNjtSpan(njt_http_request_t* req) {
  if (!IsOtelEnabled(req)) {
    return NJT_DECLINED;
  }

  TraceContext* context = GetTraceContext(req);

  if (!context) {
    return NJT_DECLINED;
  }

  auto span = context->request_span;
  span->SetAttribute("http.status_code", req->headers_out.status);

  OtelNjtLocationConf* locConf = GetOtelLocationConf(req);
  if(locConf == nullptr){
    return NJT_DECLINED;
  }

  if (locConf->captureHeaders) {
    OtelCaptureHeaders(span, njt_string("http.response.header."), &req->headers_out.headers,
#if (NJT_PCRE)
                       locConf->sensitiveHeaderNames, locConf->sensitiveHeaderValues,
#endif
                       {});
  }

  AddScriptAttributes(span.get(), GetOtelMainConf(req)->scriptAttributes, req);
  AddScriptAttributes(span.get(), locConf->customAttributes, req);

  span->UpdateName(GetOperationName(req));

  span->End();
  return NJT_DECLINED;
}

static njt_int_t InitModule(njt_conf_t* conf) {
  njt_http_core_main_conf_t* main_conf =
    (njt_http_core_main_conf_t*)njt_http_conf_get_module_main_conf(conf, njt_http_core_module);

  if (!main_conf) {
    return NJT_ERROR;
  }

  struct PhaseHandler {
    njt_http_phases phase;
    njt_http_handler_pt handler;
  };

  const PhaseHandler handlers[] = {
    {NJT_HTTP_REWRITE_PHASE, StartNjtSpan},
    {NJT_HTTP_LOG_PHASE, FinishNjtSpan},
  };

  for (const PhaseHandler& ph : handlers) {
    njt_http_handler_pt* njt_handler =
      (njt_http_handler_pt*)njt_array_push(&main_conf->phases[ph.phase].handlers);

    if (njt_handler == nullptr) {
      continue;
    }

    *njt_handler = ph.handler;
  }

  OtelMainConf* otelMainConf =
    (OtelMainConf*)njt_http_conf_get_module_main_conf(conf, njt_otel_module);

  if (!otelMainConf) {
    return NJT_ERROR;
  }

  otelMainConf->scriptAttributes = njt_array_create(
    conf->pool, sizeof(kDefaultScriptAttributes) / sizeof(kDefaultScriptAttributes[0]),
    sizeof(CompiledScriptAttribute));

  if (otelMainConf->scriptAttributes == nullptr) {
    return NJT_ERROR;
  }

  for (const auto& attrib : kDefaultScriptAttributes) {
    CompiledScriptAttribute* compiledAttrib =
      (CompiledScriptAttribute*)njt_array_push(otelMainConf->scriptAttributes);

    if (compiledAttrib == nullptr) {
      return false;
    }

    new (compiledAttrib) CompiledScriptAttribute();

    if (!CompileScriptAttribute(conf, attrib, compiledAttrib)) {
      return NJT_ERROR;
    }
  }

  return NJT_OK;
}

static void* CreateOtelMainConf(njt_conf_t* conf) {
  OtelMainConf* mainConf = (OtelMainConf*)njt_pcalloc(conf->pool, sizeof(OtelMainConf));
  new (mainConf) OtelMainConf();

  return mainConf;
}

static void* CreateOtelLocConf(njt_conf_t* conf) {
  OtelNjtLocationConf* locConf =
    (OtelNjtLocationConf*)njt_pcalloc(conf->pool, sizeof(OtelNjtLocationConf));
  new (locConf) OtelNjtLocationConf();
  return locConf;
}

static char* MergeLocConf(njt_conf_t*, void* parent, void* child) {
  OtelNjtLocationConf* prev = (OtelNjtLocationConf*)parent;
  OtelNjtLocationConf* conf = (OtelNjtLocationConf*)child;

  njt_conf_merge_value(conf->enabled, prev->enabled, 0);

  if (conf->propagationType == TracePropagationUnset) {
    if (prev->propagationType != TracePropagationUnset) {
      conf->propagationType = prev->propagationType;
    } else {
      conf->propagationType = TracePropagationW3C;
    }
  }

  njt_conf_merge_value(conf->trustIncomingSpans, prev->trustIncomingSpans, 1);

  njt_conf_merge_value(conf->captureHeaders, prev->captureHeaders, 0);

#if (NJT_PCRE)
  njt_conf_merge_ptr_value(conf->sensitiveHeaderNames, prev->sensitiveHeaderNames, nullptr);
  njt_conf_merge_ptr_value(conf->sensitiveHeaderValues, prev->sensitiveHeaderValues, nullptr);

  njt_conf_merge_ptr_value(conf->ignore_paths, prev->ignore_paths, nullptr);
#endif

  if (!prev->operationNameScript.IsEmpty() && conf->operationNameScript.IsEmpty()) {
    conf->operationNameScript = prev->operationNameScript;
  }

  if (prev->customAttributes && !conf->customAttributes) {
    conf->customAttributes = prev->customAttributes;
  } else if (prev->customAttributes && conf->customAttributes) {
    std::unordered_map<nostd::string_view, CompiledScriptAttribute> mergedAttributes;

    for (njt_uint_t i = 0; i < prev->customAttributes->nelts; i++) {
      CompiledScriptAttribute* attrib =
        &((CompiledScriptAttribute*)prev->customAttributes->elts)[i];
      mergedAttributes[FromNjtString(attrib->key.pattern)] = *attrib;
    }

    for (njt_uint_t i = 0; i < conf->customAttributes->nelts; i++) {
      CompiledScriptAttribute* attrib =
        &((CompiledScriptAttribute*)conf->customAttributes->elts)[i];
      mergedAttributes[FromNjtString(attrib->key.pattern)] = *attrib;
    }

    njt_uint_t index = 0;
    for (const auto& kv : mergedAttributes) {
      if (index == conf->customAttributes->nelts) {
        CompiledScriptAttribute* attribute =
          (CompiledScriptAttribute*)njt_array_push(conf->customAttributes);

        if (!attribute) {
          return (char*)NJT_CONF_ERROR;
        }

        *attribute = kv.second;
      } else {
        CompiledScriptAttribute* attributes =
          (CompiledScriptAttribute*)conf->customAttributes->elts;
        attributes[index] = kv.second;
      }

      index++;
    }
  }

  return NJT_CONF_OK;
}

static njt_int_t CreateOtelNjtVariables(njt_conf_t* conf) {
  for (njt_http_variable_t* v = otel_njt_variables; v->name.len; v++) {
    njt_http_variable_t* var = njt_http_add_variable(conf, &v->name, v->flags);

    if (var == nullptr) {
      return NJT_ERROR;
    }

    var->get_handler = v->get_handler;
    var->set_handler = v->set_handler;
    var->data = v->data;
    v->index = var->index = njt_http_get_variable_index(conf, &v->name);
  }

  return NJT_OK;
}

static njt_http_module_t otel_njt_http_module = {
  CreateOtelNjtVariables, /* preconfiguration */
  InitModule,             /* postconfiguration */
  CreateOtelMainConf,     /* create main conf */
  nullptr,                /* init main conf */
  nullptr,                /* create server conf */
  nullptr,                /* merge server conf */
  CreateOtelLocConf,      /* create loc conf */
  MergeLocConf,           /* merge loc conf */
};

bool CompileCommandScript(njt_conf_t* njtConf, njt_command_t*, NjtCompiledScript* script) {
  njt_str_t* value = (njt_str_t*)njtConf->args->elts;
  njt_str_t* pattern = &value[1];

  return CompileScript(njtConf, *pattern, script);
}

char* OtelNjtSetOperationNameVar(njt_conf_t* njtConf, njt_command_t* cmd, void* conf) {
  auto locationConf = (OtelNjtLocationConf*)conf;
  if (CompileCommandScript(njtConf, cmd, &locationConf->operationNameScript)) {
    return NJT_CONF_OK;
  }

  return (char*)NJT_CONF_ERROR;
}

struct HeaderPropagation {
  nostd::string_view directive;
  nostd::string_view parameter;
  nostd::string_view variable;
};

std::vector<HeaderPropagation> B3PropagationVars() {
  return {
    {"proxy_set_header", "b3", "$opentelemetry_context_b3"},
    {"fastcgi_param", "HTTP_B3", "$opentelemetry_context_b3"},
  };
}

std::vector<HeaderPropagation> OtelPropagationVars() {
  return {
    {"proxy_set_header", "traceparent", "$opentelemetry_context_traceparent"},
    {"proxy_set_header", "tracestate", "$opentelemetry_context_tracestate"},
    {"fastcgi_param", "HTTP_TRACEPARENT", "$opentelemetry_context_traceparent"},
    {"fastcgi_param", "HTTP_TRACESTATE", "$opentelemetry_context_tracestate"},
  };
}

char* OtelNjtSetPropagation(njt_conf_t* conf, njt_command_t*, void* locConf) {
  uint32_t numArgs = conf->args->nelts;

  auto locationConf = (OtelNjtLocationConf*)locConf;

  if (numArgs == 2) {
    njt_str_t* args = (njt_str_t*)conf->args->elts;
    nostd::string_view propagationType = FromNjtString(args[1]);

    if (propagationType == "b3") {
      locationConf->propagationType = TracePropagationB3;
    } else if (propagationType == "w3c") {
      locationConf->propagationType = TracePropagationW3C;
    } else {
      njt_log_error(NJT_LOG_ERR, conf->log, 0, "Unsupported propagation type");
      return (char*)NJT_CONF_ERROR;
    }
  } else {
    locationConf->propagationType = TracePropagationW3C;
  }

  std::vector<HeaderPropagation> propagationVars;
  if (locationConf->propagationType == TracePropagationB3) {
    propagationVars = B3PropagationVars();
  } else {
    propagationVars = OtelPropagationVars();
  }

  njt_array_t* oldArgs = conf->args;

  for (const HeaderPropagation& propagation : propagationVars) {
    njt_str_t args[] = {
      ToNjtString(propagation.directive),
      ToNjtString(propagation.parameter),
      ToNjtString(propagation.variable),
    };

    njt_array_t argsArray;
    njt_memzero(&argsArray, sizeof(argsArray));

    argsArray.elts = &args;
    argsArray.nelts = 3;
    conf->args = &argsArray;

    if (OtelNjtConfHandler(conf, 0) != NJT_OK) {
      conf->args = oldArgs;
      return (char*)NJT_CONF_ERROR;
    }
  }

  conf->args = oldArgs;

  return NJT_CONF_OK;
}

char* OtelNjtSetConfig(njt_conf_t* conf, njt_command_t*, void*) {
  OtelMainConf* mainConf = (OtelMainConf*)njt_http_conf_get_module_main_conf(conf, njt_otel_module);
  if(mainConf == nullptr){
    return (char*)NJT_CONF_ERROR;
  }

  njt_str_t* values = (njt_str_t*)conf->args->elts;
  njt_str_t* path = &values[1];

  if (!OtelAgentConfigLoad(
        std::string((const char*)path->data, path->len), conf->log, &mainConf->agentConfig)) {
    return (char*)NJT_CONF_ERROR;
  }

  return NJT_CONF_OK;
}

static char* OtelNjtSetCustomAttribute(njt_conf_t* conf, njt_command_t*, void* userConf) {
  OtelNjtLocationConf* locConf = (OtelNjtLocationConf*)userConf;
  if(locConf == nullptr){
    return (char*)NJT_CONF_ERROR;
  }

  if (!locConf->customAttributes) {
    locConf->customAttributes = njt_array_create(conf->pool, 1, sizeof(CompiledScriptAttribute));

    if (!locConf->customAttributes) {
      return (char*)NJT_CONF_ERROR;
    }
  }

  CompiledScriptAttribute* compiledAttribute =
    (CompiledScriptAttribute*)njt_array_push(locConf->customAttributes);

  if (!compiledAttribute) {
    return (char*)NJT_CONF_ERROR;
  }

  njt_str_t* args = (njt_str_t*)conf->args->elts;

  ScriptAttributeDeclaration attrib{args[1], args[2]};
  if (!CompileScriptAttribute(conf, attrib, compiledAttribute)) {
    return (char*)NJT_CONF_ERROR;
  }

  return NJT_CONF_OK;
}

#if (NJT_PCRE)
static njt_regex_t* NjtCompileRegex(njt_conf_t* conf, njt_str_t pattern) {
  u_char err[NJT_MAX_CONF_ERRSTR];

  njt_regex_compile_t rc;
  njt_memzero(&rc, sizeof(njt_regex_compile_t));

  rc.pool = conf->pool;
  rc.pattern = pattern;
  rc.options = NJT_REGEX_CASELESS;
  rc.err.data = err;
  rc.err.len = sizeof(err);

  if (njt_regex_compile(&rc) != NJT_OK) {
    njt_log_error(NJT_LOG_ERR, conf->log, 0, "illegal regex in %V: %V", (njt_str_t*)conf->args->elts, &rc.err);
    return nullptr;
  }

  return rc.regex;
}

static char* OtelNjtSetSensitiveHeaderNames(njt_conf_t* conf, njt_command_t*, void* userConf) {
  OtelNjtLocationConf* locConf = (OtelNjtLocationConf*)userConf;
  njt_str_t* args = (njt_str_t*)conf->args->elts;

  locConf->sensitiveHeaderNames = NjtCompileRegex(conf, args[1]);
  if (!locConf->sensitiveHeaderNames) {
    return (char*)NJT_CONF_ERROR;
  }

  return NJT_CONF_OK;
}

static char* OtelNjtSetSensitiveHeaderValues(njt_conf_t* conf, njt_command_t*, void* userConf) {
  OtelNjtLocationConf* locConf = (OtelNjtLocationConf*)userConf;
  njt_str_t* args = (njt_str_t*)conf->args->elts;

  locConf->sensitiveHeaderValues = NjtCompileRegex(conf, args[1]);
  if (!locConf->sensitiveHeaderValues) {
    return (char*)NJT_CONF_ERROR;
  }

  return NJT_CONF_OK;
}

static char* OtelNjtSetIgnorePaths(njt_conf_t* conf, njt_command_t*, void* userConf) {
  OtelNjtLocationConf* locConf = (OtelNjtLocationConf*)userConf;
  njt_str_t* args = (njt_str_t*)conf->args->elts;

  locConf->ignore_paths = NjtCompileRegex(conf, args[1]);
  if (!locConf->ignore_paths) {
    return (char*)NJT_CONF_ERROR;
  }

  return NJT_CONF_OK;
}
#endif

static njt_command_t kOtelNjtCommands[] = {
  {
    njt_string("opentelemetry_propagate"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_NOARGS | NJT_CONF_TAKE1,
    OtelNjtSetPropagation,
    NJT_HTTP_LOC_CONF_OFFSET,
    0,
    nullptr,
  },
  {
    njt_string("opentelemetry_operation_name"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_TAKE1,
    OtelNjtSetOperationNameVar,
    NJT_HTTP_LOC_CONF_OFFSET,
    0,
    nullptr,
  },
  {
    njt_string("opentelemetry_config"),
    NJT_HTTP_MAIN_CONF | NJT_CONF_TAKE1,
    OtelNjtSetConfig,
    NJT_HTTP_LOC_CONF_OFFSET,
    0,
    nullptr,
  },
  {
    njt_string("opentelemetry_attribute"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_TAKE2,
    OtelNjtSetCustomAttribute,
    NJT_HTTP_LOC_CONF_OFFSET,
    0,
    nullptr,
  },
  {
    njt_string("opentelemetry"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_TAKE1,
    njt_conf_set_flag_slot,
    NJT_HTTP_LOC_CONF_OFFSET,
    offsetof(OtelNjtLocationConf, enabled),
    nullptr,
  },
  {
    njt_string("opentelemetry_trust_incoming_spans"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_TAKE1,
    njt_conf_set_flag_slot,
    NJT_HTTP_LOC_CONF_OFFSET,
    offsetof(OtelNjtLocationConf, trustIncomingSpans),
    nullptr,
  },
  {
    njt_string("opentelemetry_capture_headers"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_TAKE1,
    njt_conf_set_flag_slot,
    NJT_HTTP_LOC_CONF_OFFSET,
    offsetof(OtelNjtLocationConf, captureHeaders),
    nullptr,
  },
#if (NJT_PCRE)
  {
    njt_string("opentelemetry_sensitive_header_names"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_TAKE1,
    OtelNjtSetSensitiveHeaderNames,
    NJT_HTTP_LOC_CONF_OFFSET,
    0,
    nullptr,
  },
  {
    njt_string("opentelemetry_sensitive_header_values"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_TAKE1,
    OtelNjtSetSensitiveHeaderValues,
    NJT_HTTP_LOC_CONF_OFFSET,
    0,
    nullptr,
  },
  {
    njt_string("opentelemetry_ignore_paths"),
    NJT_HTTP_MAIN_CONF | NJT_HTTP_SRV_CONF | NJT_HTTP_LOC_CONF | NJT_CONF_TAKE1,
    OtelNjtSetIgnorePaths,
    NJT_HTTP_LOC_CONF_OFFSET,
    0,
    nullptr,
  },
#endif
  njt_null_command,
};

static std::unique_ptr<sdktrace::SpanExporter> CreateExporter(const OtelNjtAgentConfig* conf) {
  std::unique_ptr<sdktrace::SpanExporter> exporter;

  switch (conf->exporter.type) {
    case OtelExporterOTLP: {
      //by clb
      //std::string endpoint = conf->exporter.endpoint;
      //otlp::OtlpGrpcExporterOptions opts{endpoint};
      otlp::OtlpGrpcExporterOptions opts;
      opts.endpoint = conf->exporter.endpoint;
      opts.use_ssl_credentials = conf->exporter.use_ssl_credentials;
      opts.ssl_credentials_cacert_path = conf->exporter.ssl_credentials_cacert_path;
      exporter.reset(new otlp::OtlpGrpcExporter(opts));
      break;
    }
    default:
      break;
  }

  return exporter;
}

static std::unique_ptr<sdktrace::SpanProcessor>
CreateProcessor(const OtelNjtAgentConfig* conf, std::unique_ptr<sdktrace::SpanExporter> exporter) {
  if (conf->processor.type == OtelProcessorBatch) {
    sdktrace::BatchSpanProcessorOptions opts;
    opts.max_queue_size = conf->processor.batch.maxQueueSize;
    opts.schedule_delay_millis =
      std::chrono::milliseconds(conf->processor.batch.scheduleDelayMillis);
    opts.max_export_batch_size = conf->processor.batch.maxExportBatchSize;

    return std::unique_ptr<sdktrace::SpanProcessor>(
      new sdktrace::BatchSpanProcessor(std::move(exporter), opts));
  }

  return std::unique_ptr<sdktrace::SpanProcessor>(
    new sdktrace::SimpleSpanProcessor(std::move(exporter)));
}

static std::unique_ptr<sdktrace::Sampler> CreateSampler(const OtelNjtAgentConfig* conf) {
  if (conf->sampler.parentBased) {
    std::shared_ptr<sdktrace::Sampler> sampler;

    switch (conf->sampler.type) {
      case OtelSamplerAlwaysOn: {
        sampler = std::make_shared<sdktrace::AlwaysOnSampler>();
        break;
      }
      case OtelSamplerAlwaysOff: {
        sampler = std::make_shared<sdktrace::AlwaysOffSampler>();
        break;
      }
      case OtelSamplerTraceIdRatioBased: {
        sampler = std::make_shared<sdktrace::TraceIdRatioBasedSampler>(conf->sampler.ratio);
        break;
      }
      default:
        break;
    }

    return std::unique_ptr<sdktrace::ParentBasedSampler>(new sdktrace::ParentBasedSampler(sampler));
  }

  std::unique_ptr<sdktrace::Sampler> sampler;

  switch (conf->sampler.type) {
    case OtelSamplerAlwaysOn: {
      sampler.reset(new sdktrace::AlwaysOnSampler());
      break;
    }
    case OtelSamplerAlwaysOff: {
      sampler.reset(new sdktrace::AlwaysOffSampler());
      break;
    }
    case OtelSamplerTraceIdRatioBased: {
      sampler.reset(new sdktrace::TraceIdRatioBasedSampler(conf->sampler.ratio));
      break;
    }
    default:
      break;
  }
  return sampler;
}

static njt_int_t OtelNjtStart(njt_cycle_t* cycle) {

  OtelMainConf* otelMainConf =
    (OtelMainConf*)njt_http_cycle_get_module_main_conf(cycle, njt_otel_module);
  if(otelMainConf == nullptr){
    njt_log_error(NJT_LOG_INFO, cycle->log, 0, "Unable to get njt_otel module config");
    return NJT_OK;
  }

  OtelNjtAgentConfig* agentConf = &otelMainConf->agentConfig;

  if(agentConf == nullptr){
    njt_log_error(NJT_LOG_ERR, cycle->log, 0, "Unable to get njt_otel module agent config");
    return NJT_ERROR;
  }

  auto exporter = CreateExporter(agentConf);
  if (!exporter) {
    njt_log_error(NJT_LOG_ERR, cycle->log, 0, "Unable to create span exporter - invalid type");
    return NJT_ERROR;
  }
  auto sampler = CreateSampler(agentConf);
  if (!sampler) {
    njt_log_error(NJT_LOG_ERR, cycle->log, 0, "Unable to create sampler - invalid type");
    return NJT_ERROR;
  }
  
  auto processor = CreateProcessor(agentConf, std::move(exporter));
  auto provider =
    nostd::shared_ptr<opentelemetry::trace::TracerProvider>(new sdktrace::TracerProvider(
      std::move(processor),
      opentelemetry::sdk::resource::Resource::Create({{"service.name", agentConf->service.name}}),
      std::move(sampler)));
  opentelemetry::trace::Provider::SetTracerProvider(std::move(provider));

  return NJT_OK;
}

njt_module_t njt_otel_module = {
  NJT_MODULE_V1,
  &otel_njt_http_module,
  kOtelNjtCommands,
  NJT_HTTP_MODULE,
  nullptr,      /* init master */
  nullptr,      /* init module - prior to forking from master process */
  OtelNjtStart, /* init process - worker process fork */
  nullptr,      /* init thread */
  nullptr,      /* exit thread */
  nullptr,      /* exit process - worker process exit */
  nullptr,      /* exit master */
  NJT_MODULE_V1_PADDING,
};
