/*
* Copyright 2022, OpenTelemetry Authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#include <njt_config.h>
#include <njt_core.h>
#include <njt_http.h>
#include <stdbool.h>
#include "OpentelemetrySdk.h"
#include "opentelemetry_njt_api.h"

#define LOWEST_HTTP_ERROR_CODE 400
#define STATUS_CODE_BYTE_COUNT 6
static const int CONFIG_COUNT = 17; // Number of key value pairs in config

/*  The following enum has one-to-one mapping with
    otel_monitored_modules[] defined in .c file.
    The order should match with the below indices.
    Any new handler should be added before
    NJT_HTTP_MAX_HANDLE_COUNT
*/
enum NJT_HTTP_INDEX {
    NJT_HTTP_REALIP_MODULE_INDEX=0,         // 0
    NJT_HTTP_REWRITE_MODULE_INDEX,          // 1
    NJT_HTTP_LIMIT_CONN_MODULE_INDEX,       // 2
    NJT_HTTP_LIMIT_REQ_MODULE_INDEX,        // 3
    NJT_HTTP_LIMIT_AUTH_REQ_MODULE_INDEX,   // 4
    NJT_HTTP_AUTH_BASIC_MODULE_INDEX,       // 5
    NJT_HTTP_ACCESS_MODULE_INDEX,           // 6
    NJT_HTTP_STATIC_MODULE_INDEX,           // 7
    NJT_HTTP_GZIP_STATIC_MODULE_INDEX,      // 8
    NJT_HTTP_DAV_MODULE_INDEX,              // 9
    NJT_HTTP_AUTO_INDEX_MODULE_INDEX,       // 10
    NJT_HTTP_INDEX_MODULE_INDEX,            // 11
    NJT_HTTP_RANDOM_INDEX_MODULE_INDEX,     // 12
    NJT_HTTP_LOG_MODULE_INDEX,              // 13
    NJT_HTTP_TRY_FILES_MODULE_INDEX,        // 14
    NJT_HTTP_MIRROR_MODULE_INDEX,           // 15
    NJT_HTTP_MAX_HANDLE_COUNT               // 16
}njt_http_index;


/*
Function pointer struct for module specific hooks
*/
typedef njt_int_t (*mod_handler)(njt_http_request_t*);
mod_handler h[NJT_HTTP_MAX_HANDLE_COUNT];

/*
 Structure for storing module details for mapping module handlers with their respective module
*/
typedef struct {
        char* name;
        njt_uint_t njt_index;
        njt_http_phases ph[2];
        mod_handler handler;
        njt_uint_t mod_phase_index;
        njt_uint_t phase_count;
}otel_njt_module;

/*
	Configuration struct for module
*/
typedef struct {
    njt_flag_t  njetModuleEnabled;		// OPTIONAL: ON or OFF to enable the OpenTelemetry NGINX Agent or not respectively. (defaults to true)
    njt_str_t	njetModuleOtelSpanExporter;
    njt_str_t	njetModuleOtelExporterEndpoint;
    njt_flag_t  njetModuleOtelSslEnabled;
    njt_str_t   njetModuleOtelSslCertificatePath;
    njt_str_t	njetModuleOtelSpanProcessor;
    njt_str_t	njetModuleOtelSampler;
    njt_str_t	njetModuleServiceName;
    njt_str_t   njetModuleServiceNamespace;
    njt_str_t   njetModuleServiceInstanceId;
    njt_uint_t  njetModuleOtelMaxQueueSize;
    njt_uint_t  njetModuleOtelScheduledDelayMillis;
    njt_uint_t  njetModuleOtelExportTimeoutMillis;
    njt_uint_t  njetModuleOtelMaxExportBatchSize;
    njt_flag_t  njetModuleReportAllInstrumentedModules;
    njt_flag_t	njetModuleResolveBackends;
    njt_flag_t	njetModuleTraceAsInfo;
    njt_flag_t  njetModuleMaskCookie;
    njt_flag_t  njetModuleMaskSmUser;
    njt_str_t   njetModuleCookieMatchPattern;
    njt_str_t   njetModuleDelimiter;
    njt_str_t   njetModuleSegment;
    njt_str_t   njetModuleMatchfilter;
    njt_str_t   njetModuleMatchpattern;
    njt_str_t   njetModuleSegmentType;
    njt_str_t   njetModuleSegmentParameter;
    njt_str_t   njetModuleRequestHeaders;
    njt_str_t   njetModuleResponseHeaders;
    njt_str_t   njetModuleOtelExporterOtlpHeaders;
} njt_http_opentelemetry_loc_conf_t;

/*
    Configuration structure for storing information throughout the worker process life-time
*/
typedef struct {
    njt_flag_t  isInitialized;
    char* pid;
} njt_http_opentelemetry_worker_conf_t;

typedef struct{
    const char* key;
    const char* value;
}NJT_HTTP_OTEL_RECORDS;

typedef struct {
   OTEL_SDK_HANDLE_REQ otel_req_handle_key;
   OTEL_SDK_ENV_RECORD* propagationHeaders;
   int pheaderCount;
}njt_http_otel_handles_t;

typedef struct{
    njt_str_t sNamespace;
    njt_str_t sName;
    njt_str_t sInstanceId;
}contextNode;

// static njt_http_output_header_filter_pt  njt_http_next_header_filter;
// static njt_http_output_body_filter_pt    njt_http_next_body_filter;

/* Function prototypes */
static void *njt_http_opentelemetry_create_loc_conf(njt_conf_t *cf);
static char *njt_http_opentelemetry_merge_loc_conf(njt_conf_t *cf, void *parent, void *child);
static njt_int_t njt_http_opentelemetry_init(njt_conf_t *cf);
static njt_int_t njt_http_opentelemetry_init_worker(njt_cycle_t *cycle);
static void njt_http_opentelemetry_exit_worker(njt_cycle_t *cycle);
static njt_flag_t njt_initialize_opentelemetry(njt_http_request_t *r);
static void fillRequestPayload(request_payload* req_payload, njt_http_request_t* r);
static void fillResponsePayload(response_payload* res_payload, njt_http_request_t* r);
static void startMonitoringRequest(njt_http_request_t* r);
static void stopMonitoringRequest(njt_http_request_t* r,
        OTEL_SDK_HANDLE_REQ request_handle_key);
static OTEL_SDK_STATUS_CODE otel_startInteraction(njt_http_request_t* r, const char* module_name);
static void otel_stopInteraction(njt_http_request_t* r, const char* module_name,
        OTEL_SDK_HANDLE_REQ request_handle_key);
static void otel_payload_decorator(njt_http_request_t* r, OTEL_SDK_ENV_RECORD* propagationHeaders, int count);
static njt_flag_t otel_requestHasErrors(njt_http_request_t* r);
static njt_uint_t otel_getErrorCode(njt_http_request_t* r);
static char* njt_otel_context_set(njt_conf_t *cf, njt_command_t *cmd, void *conf);
static void njt_otel_set_global_context(njt_http_opentelemetry_loc_conf_t * prev);
static void removeUnwantedHeader(njt_http_request_t* r);
/*
    Module specific handler
*/
static njt_int_t njt_http_otel_rewrite_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_limit_conn_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_limit_req_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_realip_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_auth_request_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_auth_basic_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_access_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_static_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_gzip_static_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_dav_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_autoindex_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_index_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_random_index_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_log_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_try_files_handler(njt_http_request_t *r);
static njt_int_t njt_http_otel_mirror_handler(njt_http_request_t *r);


/*
    Utility fuction to check if the given module is monitored by Opentelemetry Agent
*/

static void traceConfig(njt_http_request_t *r, njt_http_opentelemetry_loc_conf_t* conf);
static njt_int_t isOTelMonitored(const char* str);
static char* computeContextName(njt_http_request_t *r, njt_http_opentelemetry_loc_conf_t* conf);

/* Filters */
// static njt_int_t njt_http_opentelemetry_header_filter(njt_http_request_t *r);
// static njt_int_t njt_http_opentelemetry_body_filter(njt_http_request_t *r, njt_chain_t *in);

