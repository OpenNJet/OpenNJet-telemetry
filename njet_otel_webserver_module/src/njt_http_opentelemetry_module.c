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

#include "njt_http_opentelemetry_module.h"
#include "njt_http_opentelemetry_log.h"
#include <sys/types.h>
#include <unistd.h>
#include <string.h>

njt_http_opentelemetry_worker_conf_t *worker_conf;
// static contextNode contexts[5];
// static unsigned int c_count = 0;
static contextNode contexts;
// static unsigned int isGlobalContextSet = 0;
static njt_str_t hostname;

/*
List of modules being monitored
*/
otel_njt_module otel_monitored_modules[] = {
    {
        "njt_http_realip_module",
        0,
        {NJT_HTTP_POST_READ_PHASE, NJT_HTTP_PREACCESS_PHASE},
        njt_http_otel_realip_handler,
        0,
        2
    },
    {
        "njt_http_rewrite_module",
        0,
        {NJT_HTTP_SERVER_REWRITE_PHASE, NJT_HTTP_REWRITE_PHASE},
        njt_http_otel_rewrite_handler,
        0,
        2
    },
    {
        "njt_http_limit_conn_module",
        0,
        {NJT_HTTP_PREACCESS_PHASE},
        njt_http_otel_limit_conn_handler,
        0,
        1
    },
    {
        "njt_http_limit_req_module",
        0,
        {NJT_HTTP_PREACCESS_PHASE},
        njt_http_otel_limit_req_handler,
        0,
        1
    },
    {
        "njt_http_auth_request_module",
        0,
        {NJT_HTTP_ACCESS_PHASE},
        njt_http_otel_auth_request_handler,
        0,
        1
    },
    {
        "njt_http_auth_basic_module",
        0,
        {NJT_HTTP_ACCESS_PHASE},
        njt_http_otel_auth_basic_handler,
        0,
        1
    },
    {
        "njt_http_access_module",
        0,
        {NJT_HTTP_ACCESS_PHASE},
        njt_http_otel_access_handler,
        0,
        1
    },
    {
        "njt_http_static_module",
        0,
        {NJT_HTTP_CONTENT_PHASE},
        njt_http_otel_static_handler,
        0,
        1
    },
    {
        "njt_http_gzip_static_module",
        0,
        {NJT_HTTP_CONTENT_PHASE},
        njt_http_otel_gzip_static_handler,
        0,
        1
    },
    {
        "njt_http_dav_module",
        0,
        {NJT_HTTP_CONTENT_PHASE},
        njt_http_otel_dav_handler,
        0,
        1
    },
    {
        "njt_http_autoindex_module",
        0,
        {NJT_HTTP_CONTENT_PHASE},
        njt_http_otel_autoindex_handler,
        0,
        1
    },
    {
        "njt_http_index_module",
        0,
        {NJT_HTTP_CONTENT_PHASE},
        njt_http_otel_index_handler,
        0,
        1
    },
    {
        "njt_http_random_index_module",
        0,
        {NJT_HTTP_CONTENT_PHASE},
        njt_http_otel_random_index_handler,
        0,
        1
    },
    {
        "njt_http_log_module",
        0,
        {NJT_HTTP_LOG_PHASE},
        njt_http_otel_log_handler,
        0,
        1
    },
    {
        "njt_http_try_files_module",
        0,
        {NJT_HTTP_PRECONTENT_PHASE},
        njt_http_otel_try_files_handler,
        0,
        1
    },
    {
        "njt_http_mirror_module",
        0,
        {NJT_HTTP_PRECONTENT_PHASE},
        njt_http_otel_mirror_handler,
        0,
        1
    }
};


/*
	Here's the list of directives specific to our module, and information about where they
	may appear and how the command parser should process them.
*/
static njt_command_t njt_http_opentelemetry_commands[] = {

    { njt_string("NjetModuleEnabled"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_FLAG,
      njt_conf_set_flag_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleEnabled),
      NULL},

    { njt_string("NjetModuleOtelSpanExporter"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelSpanExporter),
      NULL},

    { njt_string("NjetModuleOtelSslEnabled"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_FLAG,
      njt_conf_set_flag_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelSslEnabled),
      NULL},

    { njt_string("NjetModuleOtelSslCertificatePath"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelSslCertificatePath),
      NULL},

    { njt_string("NjetModuleOtelExporterEndpoint"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelExporterEndpoint),
      NULL},

    { njt_string("NjetModuleOtelExporterOtlpHeaders"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelExporterOtlpHeaders),
      NULL},

    { njt_string("NjetModuleOtelSpanProcessor"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelSpanProcessor),
      NULL},

    { njt_string("NjetModuleOtelSampler"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelSampler),
      NULL},

    { njt_string("NjetModuleServiceName"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleServiceName),
      NULL},

    { njt_string("NjetModuleServiceNamespace"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleServiceNamespace),
      NULL},

    { njt_string("NjetModuleServiceInstanceId"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleServiceInstanceId),
      NULL},

    { njt_string("NjetModuleOtelMaxQueueSize"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_size_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelMaxQueueSize),
      NULL},

    { njt_string("NjetModuleOtelScheduledDelayMillis"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_msec_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelScheduledDelayMillis),
      NULL},

    { njt_string("NjetModuleOtelExportTimeoutMillis"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_msec_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelExportTimeoutMillis),
      NULL},

    { njt_string("NjetModuleOtelMaxExportBatchSize"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_size_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleOtelMaxExportBatchSize),
      NULL},

    { njt_string("NjetModuleResolveBackends"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_flag_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleResolveBackends),
      NULL},

    { njt_string("NjetModuleTraceAsInfo"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_flag_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleTraceAsInfo),
      NULL},

    { njt_string("NjetModuleReportAllInstrumentedModules"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_flag_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleReportAllInstrumentedModules),
      NULL},

    { njt_string("NjetModuleWebserverContext"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE3,
      njt_otel_context_set,
      NJT_HTTP_LOC_CONF_OFFSET,
      0,
      NULL},

    { njt_string("NjetModuleMaskCookie"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_flag_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleMaskCookie),
      NULL},

    { njt_string("NjetModuleCookieMatchPattern"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleCookieMatchPattern),
      NULL},

    { njt_string("NjetModuleMaskSmUser"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_flag_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleMaskSmUser),
      NULL},

    { njt_string("NjetModuleDelimiter"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleDelimiter),
      NULL},

    { njt_string("NjetModuleSegment"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleSegment),
      NULL},

    { njt_string("NjetModuleMatchfilter"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleMatchfilter),
      NULL},

    { njt_string("NjetModuleMatchpattern"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleMatchpattern),
      NULL},

    { njt_string("NjetModuleSegmentType"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleSegmentType),
      NULL},

    { njt_string("NjetModuleSegmentParameter"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleSegmentParameter),
      NULL},

    { njt_string("NjetModuleRequestHeaders"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleRequestHeaders),
      NULL},

    { njt_string("NjetModuleResponseHeaders"),
      NJT_HTTP_MAIN_CONF|NJT_HTTP_SRV_CONF|NJT_HTTP_LOC_CONF|NJT_CONF_TAKE1,
      njt_conf_set_str_slot,
      NJT_HTTP_LOC_CONF_OFFSET,
      offsetof(njt_http_opentelemetry_loc_conf_t, njetModuleResponseHeaders),
      NULL},

    njt_null_command	/* command termination */
};

/* The module context. */
static njt_http_module_t njt_otel_webserver_module_ctx = {
    NULL,						/* preconfiguration */
    njt_http_opentelemetry_init,	                        /* postconfiguration */

    NULL,	                                        /* create main configuration */
    NULL,	                                        /* init main configuration */

    NULL,	                                        /* create server configuration */
    NULL,	                                        /* merge server configuration */

    njt_http_opentelemetry_create_loc_conf,	        /* create location configuration */
    njt_http_opentelemetry_merge_loc_conf		        /* merge location configuration */
};

/* Module definition. */
njt_module_t njt_otel_webserver_module = {
    NJT_MODULE_V1,							/* module version and a signature */
    &njt_otel_webserver_module_ctx,		                        /* module context */
    njt_http_opentelemetry_commands,			                /* module directives */
    NJT_HTTP_MODULE, 						        /* module type */
    NULL, 								/* init master */
    NULL, 								/* init module */
    njt_http_opentelemetry_init_worker, 	                                /* init process */
    NULL, 								/* init thread */
    NULL, 								/* exit thread */
    njt_http_opentelemetry_exit_worker,   				/* exit process */
    NULL, 								/* exit master */
    NJT_MODULE_V1_PADDING
};

/*
	Create loc conf to be used by the module
	It takes a directive struct (njt_conf_t) and returns a newly
	created module configuration struct
 */
static void* njt_http_opentelemetry_create_loc_conf(njt_conf_t *cf)
{
    njt_http_opentelemetry_loc_conf_t  *conf;

    conf = njt_pcalloc(cf->pool, sizeof(njt_http_opentelemetry_loc_conf_t));
    if (conf == NULL) {
        return NJT_CONF_ERROR;
    }

    //zero set
    njt_memzero(&contexts, sizeof(contexts));

    /* Initialize */
    conf->njetModuleEnabled                   = NJT_CONF_UNSET;
    conf->njetModuleResolveBackends           = NJT_CONF_UNSET;
    conf->njetModuleOtelScheduledDelayMillis  = NJT_CONF_UNSET;
    conf->njetModuleOtelExportTimeoutMillis   = NJT_CONF_UNSET;
    conf->njetModuleOtelMaxExportBatchSize    = NJT_CONF_UNSET;
    conf->njetModuleReportAllInstrumentedModules = NJT_CONF_UNSET;
    conf->njetModuleMaskCookie                = NJT_CONF_UNSET;
    conf->njetModuleMaskSmUser                = NJT_CONF_UNSET;
    conf->njetModuleTraceAsInfo              = NJT_CONF_UNSET;
    conf->njetModuleOtelMaxQueueSize          = NJT_CONF_UNSET;
    conf->njetModuleOtelSslEnabled            = NJT_CONF_UNSET;

    return conf;
}

static char* njt_http_opentelemetry_merge_loc_conf(njt_conf_t *cf, void *parent, void *child)
{
    njt_http_opentelemetry_loc_conf_t *prev = parent;
    njt_http_opentelemetry_loc_conf_t *conf = child;
    // njt_otel_set_global_context(prev);

    njt_conf_merge_str_value(contexts.sNamespace, prev->njetModuleServiceNamespace, "NjetServiceNamespace");
    njt_conf_merge_str_value(contexts.sName, prev->njetModuleServiceName, "NjetService");
    njt_conf_merge_str_value(contexts.sInstanceId, prev->njetModuleServiceInstanceId, "NjetInstanceId");

    njt_conf_merge_value(conf->njetModuleEnabled, prev->njetModuleEnabled, 0);
    njt_conf_merge_value(conf->njetModuleReportAllInstrumentedModules, prev->njetModuleReportAllInstrumentedModules, 0);
    njt_conf_merge_value(conf->njetModuleResolveBackends, prev->njetModuleResolveBackends, 1);
    njt_conf_merge_value(conf->njetModuleTraceAsInfo, prev->njetModuleTraceAsInfo, 0);
    njt_conf_merge_value(conf->njetModuleMaskCookie, prev->njetModuleMaskCookie, 0);
    njt_conf_merge_value(conf->njetModuleMaskSmUser, prev->njetModuleMaskSmUser, 0);

    njt_conf_merge_str_value(conf->njetModuleOtelSpanExporter, prev->njetModuleOtelSpanExporter, "");
    njt_conf_merge_str_value(conf->njetModuleOtelExporterEndpoint, prev->njetModuleOtelExporterEndpoint, "");
    njt_conf_merge_str_value(conf->njetModuleOtelExporterOtlpHeaders, prev->njetModuleOtelExporterOtlpHeaders, "");
    njt_conf_merge_value(conf->njetModuleOtelSslEnabled, prev->njetModuleOtelSslEnabled, 0);
    njt_conf_merge_str_value(conf->njetModuleOtelSslCertificatePath, prev->njetModuleOtelSslCertificatePath, "");
    njt_conf_merge_str_value(conf->njetModuleOtelSpanProcessor, prev->njetModuleOtelSpanProcessor, "");
    njt_conf_merge_str_value(conf->njetModuleOtelSampler, prev->njetModuleOtelSampler, "");
    njt_conf_merge_str_value(conf->njetModuleServiceName, prev->njetModuleServiceName, "");
    njt_conf_merge_str_value(conf->njetModuleServiceNamespace, prev->njetModuleServiceNamespace, "");
    njt_conf_merge_str_value(conf->njetModuleServiceInstanceId, prev->njetModuleServiceInstanceId, "");
    njt_conf_merge_str_value(conf->njetModuleCookieMatchPattern, prev->njetModuleCookieMatchPattern, "");
    njt_conf_merge_str_value(conf->njetModuleDelimiter, prev->njetModuleDelimiter, "");
    njt_conf_merge_str_value(conf->njetModuleMatchfilter, prev->njetModuleMatchfilter, "");
    njt_conf_merge_str_value(conf->njetModuleSegment, prev->njetModuleSegment, "");
    njt_conf_merge_str_value(conf->njetModuleMatchpattern, prev->njetModuleMatchpattern, "");

    njt_conf_merge_size_value(conf->njetModuleOtelMaxQueueSize, prev->njetModuleOtelMaxQueueSize, 2048);
    njt_conf_merge_msec_value(conf->njetModuleOtelScheduledDelayMillis, prev->njetModuleOtelScheduledDelayMillis, 5000);
    njt_conf_merge_msec_value(conf->njetModuleOtelExportTimeoutMillis, prev->njetModuleOtelExportTimeoutMillis, 30000);
    njt_conf_merge_size_value(conf->njetModuleOtelMaxExportBatchSize, prev->njetModuleOtelMaxExportBatchSize, 512);

    njt_conf_merge_str_value(conf->njetModuleSegmentType, prev->njetModuleSegmentType, "First");
    njt_conf_merge_str_value(conf->njetModuleSegmentParameter, prev->njetModuleSegmentParameter, "2");
    njt_conf_merge_str_value(conf->njetModuleRequestHeaders, prev->njetModuleRequestHeaders, "");
    njt_conf_merge_str_value(conf->njetModuleResponseHeaders, prev->njetModuleResponseHeaders, "");

    return NJT_CONF_OK;
}

/*
	Function to initialize the module and used to register all the phases handlers and filters.
	-------------------------------------------------------------------------------------------------
	For reference: HTTP Request phases

	Each HTTP request passes through a sequence of phases. In each phase a distinct type of processing
	is performed on the request. Module-specific handlers can be registered in most phases, and many
	standard njet modules register their phase handlers as a way to get called at a specific stage of
	request processing. Phases are processed successively and the phase handlers are called once the
	request reaches the phase. Following is the list of njet HTTP phases:

	NJT_HTTP_POST_READ_PHASE
	NJT_HTTP_SERVER_REWRITE_PHASE
	NJT_HTTP_FIND_CONFIG_PHASE
	NJT_HTTP_REWRITE_PHASE
	NJT_HTTP_POST_REWRITE_PHASE
	NJT_HTTP_PREACCESS_PHASE
	NJT_HTTP_ACCESS_PHASE
	NJT_HTTP_POST_ACCESS_PHASE
	NJT_HTTP_PRECONTENT_PHASE
	NJT_HTTP_CONTENT_PHASE
	NJT_HTTP_LOG_PHASE

	On every phase you can register any number of your handlers. Exceptions are following phases:

	NJT_HTTP_FIND_CONFIG_PHASE
	NJT_HTTP_POST_ACCESS_PHASE
	NJT_HTTP_POST_REWRITE_PHASE
	NJT_HTTP_TRY_FILES_PHASE
	-------------------------------------------------------------------------------------------------
 */
static njt_int_t njt_http_opentelemetry_init(njt_conf_t *cf)
{
    njt_http_core_main_conf_t    *cmcf;
    njt_uint_t                   m, cp, ap, pap, srp, prp, rp, lp, pcp;
    njt_http_phases              ph;
    njt_uint_t                   phase_index;
    njt_int_t                    res;

    njt_writeInfo(cf->cycle->log, __func__, "Starting Opentelemetry Module init");

    cp = ap = pap = srp = prp = rp = lp = pcp = 0;

    res = -1;

    cmcf = njt_http_conf_get_module_main_conf(cf, njt_http_core_module);
    if(cmcf == NULL){
        njt_writeInfo(cf->cycle->log, __func__, "http section not found in otel webserver module");
        return NJT_OK;
    }

    njt_writeInfo(cf->cycle->log, __func__, "Registering handlers for modules in different phases");

    for (m = 0; cf->cycle->modules[m]; m++) {
        if (cf->cycle->modules[m]->type == NJT_HTTP_MODULE) {
            res = isOTelMonitored(cf->cycle->modules[m]->name);
            if(res != -1){
                otel_monitored_modules[res].njt_index = m;
                phase_index = otel_monitored_modules[res].mod_phase_index;
                while(phase_index < otel_monitored_modules[res].phase_count){
                    ph = otel_monitored_modules[res].ph[phase_index];
                    switch(ph){
                        case NJT_HTTP_POST_READ_PHASE:
                            if(prp < cmcf->phases[NJT_HTTP_POST_READ_PHASE].handlers.nelts){
                                h[res] = ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_POST_READ_PHASE].handlers.elts)[prp];
                                ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_POST_READ_PHASE].handlers.elts)[prp] = otel_monitored_modules[res].handler;
                                prp++;
                            }
                            break;

                        case NJT_HTTP_SERVER_REWRITE_PHASE:
                            if(srp < cmcf->phases[NJT_HTTP_SERVER_REWRITE_PHASE].handlers.nelts){
                                h[res] = ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_SERVER_REWRITE_PHASE].handlers.elts)[srp];
                                ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_SERVER_REWRITE_PHASE].handlers.elts)[srp] = otel_monitored_modules[res].handler;
                                srp++;
                            }
                            break;

                        case NJT_HTTP_REWRITE_PHASE:
                            if(rp < cmcf->phases[NJT_HTTP_REWRITE_PHASE].handlers.nelts){
                                h[res] = ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_REWRITE_PHASE].handlers.elts)[rp];
                                ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_REWRITE_PHASE].handlers.elts)[rp] = otel_monitored_modules[res].handler;
                                rp++;
                            }
                            break;

                        case NJT_HTTP_PREACCESS_PHASE:
                            if(pap < cmcf->phases[NJT_HTTP_PREACCESS_PHASE].handlers.nelts){
                                h[res] = ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_PREACCESS_PHASE].handlers.elts)[pap];
                                ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_PREACCESS_PHASE].handlers.elts)[pap] = otel_monitored_modules[res].handler;
                                pap++;
                            }
                            break;

                        case NJT_HTTP_ACCESS_PHASE:
                            if(ap < cmcf->phases[NJT_HTTP_ACCESS_PHASE].handlers.nelts){
                                h[res] = ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_ACCESS_PHASE].handlers.elts)[ap];
                                ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_ACCESS_PHASE].handlers.elts)[ap] = otel_monitored_modules[res].handler;
                                ap++;
                            }
                            break;

                        case NJT_HTTP_CONTENT_PHASE:
                            if(cp < cmcf->phases[NJT_HTTP_CONTENT_PHASE].handlers.nelts){
                                h[res] = ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_CONTENT_PHASE].handlers.elts)[cp];
                                ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_CONTENT_PHASE].handlers.elts)[cp] = otel_monitored_modules[res].handler;
                                cp++;
                            }
                            break;

                        case NJT_HTTP_LOG_PHASE:
                            if(lp < cmcf->phases[NJT_HTTP_LOG_PHASE].handlers.nelts){
                                h[res] = ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_LOG_PHASE].handlers.elts)[lp];
                                ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_LOG_PHASE].handlers.elts)[cp] = otel_monitored_modules[res].handler;
                                lp++;
                            }
                            break;
                        case NJT_HTTP_PRECONTENT_PHASE:
                            if(pcp < cmcf->phases[NJT_HTTP_PRECONTENT_PHASE].handlers.nelts){
                                h[res] = ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_PRECONTENT_PHASE].handlers.elts)[pcp];
                                ((njt_http_handler_pt*)cmcf->phases[NJT_HTTP_PRECONTENT_PHASE].handlers.elts)[pcp] = otel_monitored_modules[res].handler;
                                pcp++;
                            }
                            break;
                        case NJT_HTTP_FIND_CONFIG_PHASE:
                        case NJT_HTTP_POST_REWRITE_PHASE:
                        case NJT_HTTP_POST_ACCESS_PHASE:
                            break;
                    }
                    phase_index++;
                }
            }
        }
    }

	/* Register header_filter */
    // njt_http_next_header_filter = njt_http_top_header_filter;
    // njt_http_top_header_filter = njt_http_opentelemetry_header_filter;

    /* Register body_filter */
    // njt_http_next_body_filter = njt_http_top_body_filter;
    // njt_http_top_body_filter = njt_http_opentelemetry_body_filter;

    hostname = cf->cycle->hostname;
    /* hostname is extracted from the njet cycle. The attribute hostname is needed
    for OTEL spec and the only place it is available is cf->cycle
    */
    njt_writeInfo(cf->cycle->log, __func__, "Opentelemetry Module init completed!");

  return NJT_OK;
}

/*
    This function gets called when master process creates worker processes
*/
static njt_int_t njt_http_opentelemetry_init_worker(njt_cycle_t *cycle)
{
    int p = getpid();
    char * s = (char *)njt_pcalloc(cycle->pool, 6);
    sprintf(s, "%d", p);
    njt_log_error(NJT_LOG_INFO, cycle->log, 0, "mod_opentelemetry: njt_http_opentelemetry_init_worker: Initializing Njet Worker for process with PID: %s", s);

    /* Allocate memory for worker configuration */
    worker_conf = njt_pcalloc(cycle->pool, sizeof(njt_http_opentelemetry_worker_conf_t));
    if (worker_conf == NULL) {
       njt_log_error(NJT_LOG_ERR, cycle->log, 0, "mod_opentelemetry: njt_http_opentelemetry_init_worker: Not able to allocate memeory for worker conf");
       return NJT_ERROR;
    }

    worker_conf->pid = s;

    return NJT_OK;
}

/*
    This function gets called when a worker process pool is destroyed
*/
static void njt_http_opentelemetry_exit_worker(njt_cycle_t *cycle)
{
    if (worker_conf && worker_conf->isInitialized)
    {
        //shoud free log and core
        releaseDependency(); 
        njt_log_error(NJT_LOG_INFO, cycle->log, 0, "mod_opentelemetry: njt_http_opentelemetry_exit_worker: Exiting Njet Worker for process with PID: %s**********", worker_conf->pid);
    }
}

static char* njt_otel_context_set(njt_conf_t *cf, njt_command_t *cmd, void *conf){
    njt_str_t* value;

    value = cf->args->elts;
    njt_http_opentelemetry_loc_conf_t * otel_conf_temp=(njt_http_opentelemetry_loc_conf_t *)conf;
    if(cf->args->nelts == 4){
        contexts.sNamespace = value[1];
        otel_conf_temp->njetModuleServiceNamespace = value[1];
        contexts.sName = value[2];
        otel_conf_temp->njetModuleServiceName = value[2];
        contexts.sInstanceId = value[3];
        otel_conf_temp->njetModuleServiceInstanceId = value[3];
    }

    return NJT_CONF_OK;
}
// static void njt_otel_set_global_context(njt_http_opentelemetry_loc_conf_t * prev)
// {
//     if(isGlobalContextSet==0){
//       if((prev->njetModuleServiceName).data != NULL && (prev->njetModuleServiceNamespace).data != NULL && (prev->njetModuleServiceInstanceId).data != NULL){
//         // isGlobalContextSet = 1;
//         contexts.sNamespace = prev->njetModuleServiceNamespace;
//         contexts.sName = prev->njetModuleServiceName;
//         contexts.sInstanceId = prev->njetModuleServiceInstanceId;
//       }
//     }
// }

/*
    Begin a new interaction
*/
static OTEL_SDK_STATUS_CODE otel_startInteraction(njt_http_request_t* r, const char* module_name){
    OTEL_SDK_STATUS_CODE res = OTEL_SUCCESS;
    njt_http_otel_handles_t* ctx;

    if(!r || r->internal)
    {
        njt_writeTrace(r->connection->log, __func__, "It is not a main Request, not starting interaction");
        return res;
    }

    ctx = njt_http_get_module_ctx(r, njt_otel_webserver_module);
    if(ctx && ctx->otel_req_handle_key)
    {
        njt_flag_t resolveBackends = false;
        njt_http_opentelemetry_loc_conf_t* conf;
        conf = njt_http_get_module_loc_conf(r, njt_otel_webserver_module);
        //add enable check
        if(!conf || !conf->njetModuleEnabled)
        {
            return res;
        }

        if(conf)
        {
            resolveBackends = conf->njetModuleResolveBackends;
        }
        OTEL_SDK_ENV_RECORD* propagationHeaders = njt_pcalloc(r->pool, 5 * sizeof(OTEL_SDK_ENV_RECORD));
        if (propagationHeaders == NULL)
        {
            njt_writeError(r->connection->log, __func__, "Failed to allocate memory for propagation headers");
            return OTEL_STATUS(fail);
        }
        njt_writeTrace(r->connection->log, __func__, "Starting a new module interaction for: %s", module_name);
        int ix = 0;
        res = startModuleInteraction((void*)ctx->otel_req_handle_key, module_name, "", resolveBackends, propagationHeaders, &ix);

        if (OTEL_ISSUCCESS(res))
        {
            removeUnwantedHeader(r);
            otel_payload_decorator(r, propagationHeaders, ix);
            njt_writeTrace(r->connection->log, __func__, "Interaction begin successful");
        }
        else
        {
            njt_writeError(r->connection->log, __func__, "Error: Interaction begin result code: %d", res);
        }
        for(int i=0;i<ix;i++)
        {
          if(propagationHeaders[i].name)
            free(propagationHeaders[i].name);
          if(propagationHeaders[i].value)
            free(propagationHeaders[i].value);
        }
    }
    return res;
}

static void otel_payload_decorator(njt_http_request_t* r, OTEL_SDK_ENV_RECORD* propagationHeaders, int count)
{
   njt_list_part_t  *part;
   njt_table_elt_t  *header;
   njt_table_elt_t            *h;
   njt_http_header_t          *hh;
   njt_http_core_main_conf_t  *cmcf;
   njt_uint_t       nelts;

   part = &r->headers_in.headers.part;
   header = (njt_table_elt_t*)part->elts;
   nelts = part->nelts;

   for(int i=0; i<count; i++){

       int header_found=0;
       for(njt_uint_t j = 0; j<nelts; j++){
           h = &header[j];
           if(strcmp(httpHeaders[i], h->key.data)==0){
               
               header_found=1;

               if(h->key.data)
                    njt_pfree(r->pool, h->key.data);
               if(h->value.data)
                    njt_pfree(r->pool, h->value.data);
               
               break;
           }
       }
       if(header_found==0)
       {
           h = njt_list_push(&r->headers_in.headers);
       }

       if(h == NULL )
            return;

       h->key.len = strlen(propagationHeaders[i].name);
       h->key.data = njt_pcalloc(r->pool, sizeof(char)*((h->key.len)+1));
       strcpy(h->key.data, propagationHeaders[i].name);

       njt_writeTrace(r->connection->log, __func__, "Key : %s", propagationHeaders[i].name);

       h->hash = njt_hash_key(h->key.data, h->key.len);

       h->value.len = strlen(propagationHeaders[i].value);
       h->value.data = njt_pcalloc(r->pool, sizeof(char)*((h->value.len)+1));
       strcpy(h->value.data, propagationHeaders[i].value);
       h->lowcase_key = h->key.data;

       cmcf = njt_http_get_module_main_conf(r, njt_http_core_module);
       if(cmcf != NULL){
            hh = njt_hash_find(&cmcf->headers_in_hash, h->hash,h->lowcase_key, h->key.len);
            if (hh && hh->handler(r, h, hh->offset) != NJT_OK) {
                return;
            }
       }

       njt_writeTrace(r->connection->log, __func__, "Value : %s", propagationHeaders[i].value);

   }
   
   njt_http_otel_handles_t* ctx = njt_http_get_module_ctx(r, njt_otel_webserver_module);
   if(ctx != NULL){
        ctx->propagationHeaders = propagationHeaders;
        ctx->pheaderCount = count;
   }
}

/*
    Stopping an Interaction
*/
static void otel_stopInteraction(njt_http_request_t* r, const char* module_name,
    void* request_handle_key)
{
    if(!r || r->internal)
    {
        return;
    }

    //add enable check
    njt_http_opentelemetry_loc_conf_t* conf;
    conf = njt_http_get_module_loc_conf(r, njt_otel_webserver_module);
    if(!conf || !conf->njetModuleEnabled)
    {
        return;
    }


    OTEL_SDK_HANDLE_REQ otel_req_handle_key = OTEL_SDK_NO_HANDLE;
    njt_http_otel_handles_t* ctx = njt_http_get_module_ctx(r, njt_otel_webserver_module);
    if (r->pool == NULL && request_handle_key != NULL)
    {
        otel_req_handle_key = request_handle_key;
    }
    else if (ctx && ctx->otel_req_handle_key)
    {
        otel_req_handle_key = ctx->otel_req_handle_key;
    }
    else
    {
        return;
    }

    // TODO: Work on backend naming and type
    char* backendName = (char *)malloc(6 * sizeof(char));
    *backendName = '\0';
    const char* backendType = "HTTP";
    unsigned int errCode=0;
    char* code = (char *)malloc(6 * sizeof(char));

    const char* status = "HTTP Status Code: ";
    char* msg = (char *)malloc(strlen(status) + 6 * sizeof(char));
    *msg = '\0';
    if(otel_requestHasErrors(r))
    {
        errCode=(unsigned int)otel_getErrorCode(r);
        sprintf(code, "%d", errCode);
        strcpy(msg, status);
        strcat(msg, code);
    }
    njt_writeTrace(r->connection->log, __func__, "Stopping the Interaction for: %s", module_name);
    OTEL_SDK_STATUS_CODE res = stopModuleInteraction(otel_req_handle_key, backendName, backendType, errCode, msg);
    if (OTEL_ISFAIL(res))
    {
        njt_writeError(r->connection->log, __func__, "Error: Stop Interaction failed, result code: %d", res);
    }

    free (backendName);
    free (code);
    free (msg);
}

static njt_flag_t otel_requestHasErrors(njt_http_request_t* r)
{
    return (r->err_status >= LOWEST_HTTP_ERROR_CODE)||(r->headers_out.status >= LOWEST_HTTP_ERROR_CODE);
}
static njt_uint_t otel_getErrorCode(njt_http_request_t* r)
{
    if(r->err_status >= LOWEST_HTTP_ERROR_CODE)
      return r->err_status;
    else if(r->headers_out.status >= LOWEST_HTTP_ERROR_CODE)
      return r->headers_out.status;
    else return 0;
}

static njt_flag_t njt_initialize_opentelemetry(njt_http_request_t *r)
{
    // check to see if we have already been initialized
    if (worker_conf && worker_conf->isInitialized)
    {
        njt_writeTrace(r->connection->log, __func__, "Opentelemetry SDK already initialized for process with PID: %s", worker_conf->pid);
        return true;
    }

    njt_http_opentelemetry_loc_conf_t	*conf;
    conf = njt_http_get_module_loc_conf(r, njt_otel_webserver_module);
    if (conf == NULL)
    {
        njt_writeError(r->connection->log, __func__, "Module location configuration is NULL");
        return false;
    }

    traceConfig(r, conf);

    if (conf->njetModuleEnabled)
    {
        OTEL_SDK_STATUS_CODE res = OTEL_SUCCESS;
        char            *qs = (char *)malloc(6);
        char            *et = (char *)malloc(6);
        char            *es = (char *)malloc(6);
        char            *sd = (char *)malloc(6);
        njt_uint_t      i;

        logState = conf->njetModuleTraceAsInfo; //Setting Logging Flag

        initDependency();

        struct cNode *cn = njt_pcalloc(r->pool, sizeof(struct cNode));
        // (cn->cInfo).cName = computeContextName(r, conf);
        struct cNode *rootCN = NULL;
        cn = NULL;


        // Update the apr_pcalloc if we add another parameter to the input array!
        OTEL_SDK_ENV_RECORD* env_config = njt_pcalloc(r->pool, CONFIG_COUNT * sizeof(OTEL_SDK_ENV_RECORD));
        if(env_config == NULL)
        {
            njt_writeError(r->connection->log, __func__, "Not Able to allocate memory for the Env Config");
            return false;
        }
        int ix = 0;

        // Otel Exporter Type
        env_config[ix].name = OTEL_SDK_ENV_OTEL_EXPORTER_TYPE;
        env_config[ix].value = (const char*)((conf->njetModuleOtelSpanExporter).data);
        ++ix;

        // sdk libaray name
        env_config[ix].name = OTEL_SDK_ENV_OTEL_LIBRARY_NAME;
        env_config[ix].value = "Njet";
        ++ix;

        // Otel Exporter Endpoint
        env_config[ix].name = OTEL_SDK_ENV_OTEL_EXPORTER_ENDPOINT;
        env_config[ix].value = (const char*)(conf->njetModuleOtelExporterEndpoint).data;
        ++ix;

        // Otel Exporter OTEL headers
        env_config[ix].name = OTEL_SDK_ENV_OTEL_EXPORTER_OTLPHEADERS;
        env_config[ix].value = (const char*)(conf->njetModuleOtelExporterOtlpHeaders).data;
        ++ix;

        // Otel SSL Enabled
        env_config[ix].name = OTEL_SDK_ENV_OTEL_SSL_ENABLED;
        env_config[ix].value = conf->njetModuleOtelSslEnabled == 1 ? "1" : "0";
        ++ix;

        // Otel SSL Certificate Path
        env_config[ix].name = OTEL_SDK_ENV_OTEL_SSL_CERTIFICATE_PATH;
        env_config[ix].value = (const char*)(conf->njetModuleOtelSslCertificatePath).data;
        ++ix;

        // Otel Processor Type
        env_config[ix].name = OTEL_SDK_ENV_OTEL_PROCESSOR_TYPE;
        env_config[ix].value = (const char*)(conf->njetModuleOtelSpanProcessor).data;
        ++ix;

        // Otel Sampler Type
        env_config[ix].name = OTEL_SDK_ENV_OTEL_SAMPLER_TYPE;
        env_config[ix].value = (const char*)(conf->njetModuleOtelSampler).data;
        ++ix;

        // Service Namespace
        env_config[ix].name = OTEL_SDK_ENV_SERVICE_NAMESPACE;
        env_config[ix].value = (const char*)(conf->njetModuleServiceNamespace).data;
        ++ix;

        // Service Name
        env_config[ix].name = OTEL_SDK_ENV_SERVICE_NAME;
        env_config[ix].value = (const char*)(conf->njetModuleServiceName).data;
        ++ix;

        // Service Instance ID
        env_config[ix].name = OTEL_SDK_ENV_SERVICE_INSTANCE_ID;
        env_config[ix].value = (const char*)(conf->njetModuleServiceInstanceId).data;
        ++ix;

        // Otel Max Queue Size
        env_config[ix].name = OTEL_SDK_ENV_MAX_QUEUE_SIZE;
        sprintf(qs, "%lu", conf->njetModuleOtelMaxQueueSize);
        env_config[ix].value = qs;
        ++ix;

        // Otel Scheduled Delay
        env_config[ix].name = OTEL_SDK_ENV_SCHEDULED_DELAY;
        sprintf(sd, "%lu", conf->njetModuleOtelScheduledDelayMillis);
        env_config[ix].value = sd;
        ++ix;

        // Otel Max Export Batch Size
        env_config[ix].name = OTEL_SDK_ENV_EXPORT_BATCH_SIZE;
        sprintf(es, "%lu", conf->njetModuleOtelMaxExportBatchSize);
        env_config[ix].value = es;
        ++ix;

        // Otel Export Timeout
        env_config[ix].name = OTEL_SDK_ENV_EXPORT_TIMEOUT;
        sprintf(et, "%lu", conf->njetModuleOtelExportTimeoutMillis);
        env_config[ix].value = et;
        ++ix;

        // Segment Type
        env_config[ix].name = OTEL_SDK_ENV_SEGMENT_TYPE;
        env_config[ix].value = (const char*)(conf->njetModuleSegmentType).data;
        ++ix;

        // Segment Parameter
        env_config[ix].name = OTEL_SDK_ENV_SEGMENT_PARAMETER;
        env_config[ix].value = (const char*)(conf->njetModuleSegmentParameter).data;
        ++ix;


        // !!!
        // Remember to update the njt_pcalloc call size if we add another parameter to the input array!
        // !!!

        // Adding the webserver context here
        for(int context_i=0; context_i<1; context_i++){
            struct cNode *temp_cn  = njt_pcalloc(r->pool, sizeof(struct cNode));
	    char* name = njt_pcalloc(r->pool,(contexts.sNamespace).len + (contexts.sName).len + (contexts.sInstanceId).len + 1);
            if(name != NULL){
                strcpy(name, (const char*)(contexts.sNamespace).data);
                strcat(name, (const char*)(contexts.sName).data);
                strcat(name, (const char*)(contexts.sInstanceId).data);
            }
            (temp_cn->cInfo).cName = name;
            (temp_cn->cInfo).sNamespace = (const char*)(contexts.sNamespace).data;
            (temp_cn->cInfo).sName = (const char*)(contexts.sName).data;
            (temp_cn->cInfo).sInstanceId = (const char*)(contexts.sInstanceId).data;
            if(context_i==0)
            {
              cn = temp_cn;
              rootCN = cn;
            }
            else
            {
              cn->next = temp_cn;
              cn = cn->next;
            }
        }
        setRequestResponseHeaders((const char*)(conf->njetModuleRequestHeaders).data,
           (const char*)(conf->njetModuleResponseHeaders).data);
        res = opentelemetry_core_init(env_config, ix, rootCN, njt_cycle->conf_prefix.data, njt_cycle->conf_prefix.len);
        free(qs);
        free(sd);
        free(et);
        free(es);
        if (OTEL_ISSUCCESS(res))
        {
            worker_conf->isInitialized = 1;
            njt_writeTrace(r->connection->log, __func__, "Initializing Agent Core succceeded for process with PID: %s", worker_conf->pid);
            return true;
        }
        else
        {
           njt_writeError(r->connection->log, __func__, "Agent Core Init failed, result code is %d", res);
           return false;
        }
    }
    else
    {
        // Agent core is not enabled
        njt_writeTrace(r->connection->log, __func__, "Agent Core is not enabled");
        return false;
    }
    return false;
}

static void stopMonitoringRequest(njt_http_request_t* r,
    OTEL_SDK_HANDLE_REQ request_handle_key)
{
    njt_http_opentelemetry_loc_conf_t  *njt_conf = njt_http_get_module_loc_conf(r, njt_otel_webserver_module);
    if(njt_conf == NULL){
        njt_writeTrace(r->connection->log, __func__, "otel webserver module is null");
        return;   
    }
    if(!njt_conf->njetModuleEnabled)
    {
        njt_writeTrace(r->connection->log, __func__, "Agent is Disabled");
        return;
    }

    OTEL_SDK_HANDLE_REQ otel_req_handle_key = OTEL_SDK_NO_HANDLE;
    njt_http_otel_handles_t* ctx = njt_http_get_module_ctx(r, njt_otel_webserver_module);
    if (r->pool == NULL && request_handle_key != NULL)
    {
        otel_req_handle_key = request_handle_key;
    }
    else if (ctx && ctx->otel_req_handle_key)
    {
        otel_req_handle_key = ctx->otel_req_handle_key;
    }
    else
    {
        return;
    }

    if (r->pool) {
        njt_pfree(r->pool, ctx);
    }

    njt_writeTrace(r->connection->log, __func__, "Stopping the Request Monitoring");

    response_payload* res_payload = NULL;
    if (r->pool) {
        res_payload = njt_pcalloc(r->pool, sizeof(response_payload));
        res_payload->response_headers_count = 0;
        fillResponsePayload(res_payload, r);
    }

    OTEL_SDK_STATUS_CODE res;
    char* msg = NULL;

    if (otel_requestHasErrors(r))
    {
        res_payload->status_code = (unsigned int)otel_getErrorCode(r);
        msg = (char*)malloc(STATUS_CODE_BYTE_COUNT * sizeof(char));
        sprintf(msg, "%d", res_payload->status_code);
        res = endRequest(otel_req_handle_key, msg, res_payload);
    }
    else
    {
        res_payload->status_code = r->headers_out.status;
        res = endRequest(otel_req_handle_key, msg, res_payload);
    }

    if (OTEL_ISSUCCESS(res))
    {
        njt_writeTrace(r->connection->log, __func__, "Request Ends with result code: %d", res);
    }
    else
    {
        njt_writeError(r->connection->log, __func__, "Request End FAILED with code: %d", res);
    }
    if(msg){
        free(msg);
    }
    
    return;
}

static void startMonitoringRequest(njt_http_request_t* r){
    // If a not a the main request(sub-request or internal redirect), calls Realip handler and return
    if(r->internal)
    {
        njt_writeTrace(r->connection->log, __func__, "Not a Main Request(sub-request or internal redirect)");
        return;
    }
    else if (!njt_initialize_opentelemetry(r))    /* check if Otel Agent Core is initialized */
    {
        njt_writeTrace(r->connection->log, __func__, "Opentelemetry Agent Core did not get initialized");
        return;
    }

    njt_http_otel_handles_t* ctx;
    ctx = njt_http_get_module_ctx(r, njt_otel_webserver_module);
    if(ctx && ctx->otel_req_handle_key){
        return;
    }

    njt_writeTrace(r->connection->log, __func__, "Starting Request Monitoring for: %s", r->uri.data);

    // Handle request for static contents (Njet is used for habdling static contents)

    OTEL_SDK_STATUS_CODE res = OTEL_SUCCESS;
    OTEL_SDK_HANDLE_REQ reqHandle = OTEL_SDK_NO_HANDLE;

    const char* wscontext = NULL;

    njt_http_opentelemetry_loc_conf_t  *njt_conf = njt_http_get_module_loc_conf(r, njt_otel_webserver_module);
    //add enable check
    if(!njt_conf || !njt_conf->njetModuleEnabled)
    {
        return ;
    }
    if(njt_conf)
    {
        wscontext = computeContextName(r, njt_conf);
    }

    if(wscontext)
    {
        njt_writeTrace(r->connection->log, __func__, "WebServer Context: %s", wscontext);
    }
    else
    {
        njt_writeTrace(r->connection->log, __func__, "Using Default context ");
    }

    // Fill the Request payload information and start the request monitoring
    request_payload* req_payload = njt_pcalloc(r->pool, sizeof(request_payload));
    if(req_payload == NULL)
    {
        njt_writeError(r->connection->log, __func__, "Not able to get memory for request payload");
    }
    fillRequestPayload(req_payload, r);

    res = startRequest(wscontext, req_payload, &reqHandle);

    if (OTEL_ISSUCCESS(res))
    {
        if (ctx == NULL)
        {
            ctx = njt_pcalloc(r->pool, sizeof(njt_http_otel_handles_t));
            if (ctx == NULL)
            {
                njt_writeError(r->connection->log, __func__, "Cannot allocate memory for handles");
                return;
            }
            // Store the Request Handle on the request object
            OTEL_SDK_HANDLE_REQ reqHandleValue = njt_pcalloc(r->pool, sizeof(OTEL_SDK_HANDLE_REQ));
            if (reqHandleValue)
            {
                reqHandleValue = reqHandle;
                ctx->otel_req_handle_key = reqHandleValue;
                njt_http_set_ctx(r, ctx, njt_otel_webserver_module);
            }
        }
        njt_writeTrace(r->connection->log, __func__, "Request Monitoring begins successfully ");
    }
    else if (res == OTEL_STATUS(cfg_channel_uninitialized) || res == OTEL_STATUS(bt_detection_disabled))
    {
        njt_writeTrace(r->connection->log, __func__, "Request begin detection disabled, result code: %d", res);
    }
    else
    {
        njt_writeError(r->connection->log, __func__, "Request begin error, result code: %d", res);
    }
}

static njt_int_t njt_http_otel_rewrite_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_rewrite_module");
    njt_int_t rvalue = h[NJT_HTTP_REWRITE_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_rewrite_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_limit_conn_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_limit_conn_module");
    njt_int_t rvalue = h[NJT_HTTP_LIMIT_CONN_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_limit_conn_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_limit_req_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_limit_req_module");
    njt_int_t rvalue = h[NJT_HTTP_LIMIT_REQ_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_limit_req_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_realip_handler(njt_http_request_t *r){

    // This will be the first hanndler to be encountered,
    // Here, Init and start the Request Processing by creating Trace, spans etc
    if(!r->internal)
    {
        startMonitoringRequest(r);
    }

    otel_startInteraction(r, "njt_http_realip_module");
    njt_int_t rvalue = h[NJT_HTTP_REALIP_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_realip_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_auth_request_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_auth_request_module");
    njt_int_t rvalue = h[NJT_HTTP_LIMIT_AUTH_REQ_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_auth_request_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_auth_basic_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_auth_basic_module");
    njt_int_t rvalue = h[NJT_HTTP_AUTH_BASIC_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_auth_basic_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_access_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_access_module");
    njt_int_t rvalue = h[NJT_HTTP_ACCESS_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_access_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_static_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_static_module");
    njt_int_t rvalue = h[NJT_HTTP_STATIC_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_static_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_gzip_static_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_gzip_static_module");
    njt_int_t rvalue = h[NJT_HTTP_GZIP_STATIC_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_gzip_static_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_dav_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_dav_module");
    njt_int_t rvalue = h[NJT_HTTP_DAV_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_dav_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_autoindex_handler(njt_http_request_t *r){
    otel_startInteraction(r, "njt_http_autoindex_module");
    njt_int_t rvalue = h[NJT_HTTP_AUTO_INDEX_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_autoindex_module", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

/*  autoindex, index and randomindex handlers get called during
    internal redirection. In case of index and randomIndex handlers,
    it has been observed that 'ctx' ptr gets cleaned up from request
    pool and r->internal gets set to 1. But, we need 'ctx' to stop the
    interaction.
    Therefore, as a special handling, we store the ctx pointer and reset
    before stopping the interaction. We avoid starting/stopping
    interaction/request when r->intenal is 1. But in this case,
    when we started the interaction r->internal is 0 but when the
    actual handler calls completes, it internally transforms r->internal
    to 1. Therefore, we need extra handling for r->internal as well.
*/

static njt_int_t njt_http_otel_index_handler(njt_http_request_t *r){
    bool old_internal = r->internal;
    njt_http_otel_handles_t* old_ctx;

    otel_startInteraction(r, "njt_http_index_module");
    old_ctx = njt_http_get_module_ctx(r, njt_otel_webserver_module);
    njt_int_t rvalue = h[NJT_HTTP_INDEX_MODULE_INDEX](r);
    bool new_internal = r->internal;
    if (new_internal == true && new_internal != old_internal) {
        njt_http_set_ctx(r, old_ctx, njt_otel_webserver_module);
        r->internal = 0;
        otel_stopInteraction(r, "njt_http_index_module", OTEL_SDK_NO_HANDLE);
        r->internal = 1;
    } else {
        otel_stopInteraction(r, "njt_http_index_module", OTEL_SDK_NO_HANDLE);
    }

    return rvalue;
}

static njt_int_t njt_http_otel_random_index_handler(njt_http_request_t *r){
    bool old_internal = r->internal;
    njt_http_otel_handles_t* old_ctx;

    old_ctx = njt_http_get_module_ctx(r, njt_otel_webserver_module);
    otel_startInteraction(r, "njt_http_random_index_module");
    njt_int_t rvalue = h[NJT_HTTP_RANDOM_INDEX_MODULE_INDEX](r);
    bool new_internal = r->internal;
    if (new_internal == true && new_internal != old_internal) {
        njt_http_set_ctx(r, old_ctx, njt_otel_webserver_module);
        r->internal = 0;
        otel_stopInteraction(r, "njt_http_random_index_module", OTEL_SDK_NO_HANDLE);
        r->internal = 1;

    } else {
        otel_stopInteraction(r, "njt_http_random_index_module", OTEL_SDK_NO_HANDLE);
    }

    return rvalue;
}

/*  tryfiles handler gets called with try_files tag. In this case
    also, internal redirection happens. But on completion of this
    handler, request pool gets wiped out and since ctx is created
    in request pool, its no longer valid. However, request hanlde
    is not created in request pool. Therefore,we save the request
    handle and pass it along to stop the interaction.  Also,   we
    stop the request using the same request handle.
*/


static njt_int_t njt_http_otel_try_files_handler(njt_http_request_t *r) {
    bool old_internal = r->internal;
    njt_http_otel_handles_t* old_ctx;

    OTEL_SDK_HANDLE_REQ request_handle = OTEL_SDK_NO_HANDLE;
    old_ctx = njt_http_get_module_ctx(r, njt_otel_webserver_module);
    if (old_ctx && old_ctx->otel_req_handle_key) {
        request_handle = old_ctx->otel_req_handle_key;
    }

    otel_startInteraction(r, "njt_http_otel_try_files_handler");
    njt_int_t rvalue = h[NJT_HTTP_TRY_FILES_MODULE_INDEX](r);
    bool new_internal = r->internal;
    if (new_internal == true && new_internal != old_internal) {
        r->internal = 0;
        otel_stopInteraction(r, "njt_http_otel_try_files_handler", request_handle);
        r->internal = 1;
    } else {
        otel_stopInteraction(r, "njt_http_otel_try_files_handler", OTEL_SDK_NO_HANDLE);
    }

    if (!r->pool) {
        stopMonitoringRequest(r, request_handle);
    }
    return rvalue;
}

static njt_int_t njt_http_otel_mirror_handler(njt_http_request_t *r) {
    otel_startInteraction(r, "njt_http_otel_mirror_handler");
    njt_int_t rvalue = h[NJT_HTTP_MIRROR_MODULE_INDEX](r);
    otel_stopInteraction(r, "njt_http_otel_mirror_handler", OTEL_SDK_NO_HANDLE);

    return rvalue;
}

static njt_int_t njt_http_otel_log_handler(njt_http_request_t *r){
    //This will be last handler to be be encountered before a request ends and response is finally sent back to client
    // Here, End the main trace, span created by Webserver Agent and the collected data will be passed to the backend
    // It will work as njt_http_opentelemetry_log_transaction_end
    stopMonitoringRequest(r, OTEL_SDK_NO_HANDLE);

    njt_int_t rvalue = h[NJT_HTTP_LOG_MODULE_INDEX](r);

    return rvalue;
}

static njt_int_t isOTelMonitored(const char* str){
    unsigned int i = 0;
    for(i=0; i<NJT_HTTP_MAX_HANDLE_COUNT; i++){
        if(strcmp(str, otel_monitored_modules[i].name) == 0)
            return i;
        }
    return -1;
}

static char* computeContextName(njt_http_request_t *r, njt_http_opentelemetry_loc_conf_t* conf){
    char* name = njt_pcalloc(r->pool,(conf->njetModuleServiceNamespace).len + (conf->njetModuleServiceName).len + (conf->njetModuleServiceInstanceId).len + 1);

    if(name != NULL){
        strcpy(name, (const char*)(conf->njetModuleServiceNamespace).data);
        strcat(name, (const char*)(conf->njetModuleServiceName).data);
        strcat(name, (const char*)(conf->njetModuleServiceInstanceId).data);
    }
    return name;
}

static void traceConfig(njt_http_request_t *r, njt_http_opentelemetry_loc_conf_t* conf){
    njt_writeTrace(r->connection->log, __func__, " Config { :"
                                                      "(Enabled=\"%ld\")"
                                                      "(OtelExporterEndpoint=\"%s\")"
                                                      "(OtelExporterOtlpHeader=\"%s\")"
                                                      "(OtelSslEnabled=\"%ld\")"
                                                      "(OtelSslCertificatePath=\"%s\")"
                                                      "(OtelSpanExporter=\"%s\")"
                                                      "(OtelSpanProcessor=\"%s\")"
                                                      "(OtelSampler=\"%s\")"
                                                      "(ServiceNamespace=\"%s\")"
                                                      "(ServiceName=\"%s\")"
                                                      "(ServiceInstanceId=\"%s\")"
                                                      "(OtelMaxQueueSize=\"%lu\")"
                                                      "(OtelScheduledDelayMillis=\"%lu\")"
                                                      "(OtelExportTimeoutMillis=\"%lu\")"
                                                      "(OtelMaxExportBatchSize=\"%lu\")"
                                                      "(ResolveBackends=\"%ld\")"
                                                      "(TraceAsError=\"%ld\")"
                                                      "(ReportAllInstrumentedModules=\"%ld\")"
                                                      "(MaskCookie=\"%ld\")"
                                                      "(MaskSmUser=\"%ld\")"
                                                      "(SegmentType=\"%s\")"
                                                      "(SegmentParameter=\"%s\")"
                                                      " }",
                                                      conf->njetModuleEnabled,
                                                      (conf->njetModuleOtelExporterEndpoint).data,
                                                      (conf->njetModuleOtelExporterOtlpHeaders).data,
                                                      conf->njetModuleOtelSslEnabled,
                                                      (conf->njetModuleOtelSslCertificatePath).data,
                                                      (conf->njetModuleOtelSpanExporter).data,
                                                      (conf->njetModuleOtelSpanProcessor).data,
                                                      (conf->njetModuleOtelSampler).data,
                                                      (conf->njetModuleServiceNamespace).data,
                                                      (conf->njetModuleServiceName).data,
                                                      (conf->njetModuleServiceInstanceId).data,
                                                      conf->njetModuleOtelMaxQueueSize,
                                                      conf->njetModuleOtelScheduledDelayMillis,
                                                      conf->njetModuleOtelExportTimeoutMillis,
                                                      conf->njetModuleOtelMaxExportBatchSize,
                                                      conf->njetModuleResolveBackends,
                                                      conf->njetModuleTraceAsInfo,
                                                      conf->njetModuleReportAllInstrumentedModules,
                                                      conf->njetModuleMaskCookie,
                                                      conf->njetModuleMaskSmUser,
                                                      (conf->njetModuleSegmentType).data,
                                                      (conf->njetModuleSegmentParameter).data);
}

static void removeUnwantedHeader(njt_http_request_t* r)
{
  njt_list_part_t  *part;
  njt_table_elt_t  *header;
  njt_table_elt_t            *h;
  njt_http_header_t          *hh;
  njt_http_core_main_conf_t  *cmcf;
  njt_uint_t       nelts;

  part = &r->headers_in.headers.part;
  header = (njt_table_elt_t*)part->elts;
  nelts = part->nelts;

  for(njt_uint_t j = 0; j<nelts; j++){
    h = &header[j];
    if(strcmp("singularityheader", h->key.data)==0){
      if (h->value.len == 0) {
        break;
      }
      if(h->value.data)
        njt_pfree(r->pool, h->value.data);

      char str[] = "";
      h->hash = njt_hash_key(h->key.data, h->key.len);

      h->value.len = 0;
      h->value.data = njt_pcalloc(r->pool, sizeof(char)*((h->value.len)+1));
      strcpy(h->value.data, str);
      h->lowcase_key = h->key.data;

      cmcf = njt_http_get_module_main_conf(r, njt_http_core_module);
      if(cmcf != NULL){
        hh = njt_hash_find(&cmcf->headers_in_hash, h->hash,h->lowcase_key, h->key.len);
        if (hh && hh->handler(r, h, hh->offset) != NJT_OK) {
        return;
        }
      }
      
      break;
    }
  }
}

static void fillRequestPayload(request_payload* req_payload, njt_http_request_t* r){
    njt_list_part_t  *part;
    njt_table_elt_t  *header;
    njt_uint_t       nelts;
    njt_table_elt_t  *h;

    // creating a temporary uri for uri parsing 
    // (r->uri).data has an extra component "HTTP/1.1 connection" so to obtain the uri it
    // has to trimmed. This is done by putting a '/0' after the uri length
    // WEBSRV-558
    char *temp_uri = njt_pcalloc(r->pool, (strlen((r->uri).data))+1);
    strcpy(temp_uri,(const char*)(r->uri).data);
    temp_uri[(r->uri).len]='\0';
    req_payload->uri = temp_uri;

    njt_http_core_srv_conf_t* cscf = (njt_http_core_srv_conf_t*)njt_http_get_module_srv_conf(r, njt_http_core_module);
    if(cscf == NULL){
        return;
    }
    
    req_payload->server_name = (const char*)(cscf->server_name).data;

    #if (NJT_HTTP_SSL)

      if(r->connection->ssl)
      {
        req_payload->scheme = "https";
      }
      else
      {
        req_payload->scheme = "http";
      }

    #else

      req_payload->scheme = "http";

    #endif

    // TODO - use strncpy function to just create memory of size (r->http_protocol.len)
    char *temp_http_protocol = njt_pcalloc(r->pool, (strlen((r->http_protocol).data))+1);
    strcpy(temp_http_protocol,(const char*)(r->http_protocol).data);
    temp_http_protocol[(r->http_protocol).len]='\0';
    req_payload->protocol = temp_http_protocol;

    char *temp_request_method = njt_pcalloc(r->pool, (strlen((r->method_name).data))+1);
    strcpy(temp_request_method,(const char*)(r->method_name).data);
    temp_request_method[(r->method_name).len]='\0';
    req_payload->request_method = temp_request_method;

    // flavor has to be scraped from protocol in future
    req_payload->flavor = temp_http_protocol;

    char *temp_hostname = njt_pcalloc(r->pool, (strlen(hostname.data))+1);
    strcpy(temp_hostname,(const char*)hostname.data);
    temp_hostname[hostname.len]='\0';
    req_payload->hostname = temp_hostname;

    req_payload->http_post_param = njt_pcalloc(r->pool, sizeof(u_char*));
    req_payload->http_get_param = njt_pcalloc(r->pool, sizeof(u_char*));

    if(strstr(req_payload->request_method, "GET") !=NULL){
        req_payload->http_post_param = "No param";
        if((r->args).len){
            req_payload->http_get_param = (const char*)(r->args).data;
        }else{
            req_payload->http_get_param = "No param";
        }
    }else if(strstr(req_payload->request_method, "POST") != NULL){
        req_payload->http_get_param = "No param";
        if((r->args).len){
            req_payload->http_post_param = (const char*)(r->args).data;
        }else{
            req_payload->http_post_param = "No param";
        }
    }

    req_payload->client_ip = (const char*)(r->connection->addr_text).data;
    char *temp_client_ip = njt_pcalloc(r->pool, (strlen((r->connection->addr_text).data))+1);
    strcpy(temp_client_ip,(const char*)(r->connection->addr_text).data);
    temp_client_ip[(r->connection->addr_text).len]='\0';
    req_payload->client_ip = temp_client_ip;

    njt_http_opentelemetry_loc_conf_t *conf =
      njt_http_get_module_loc_conf(r, njt_otel_webserver_module);
    part = &r->headers_in.headers.part;
    header = (njt_table_elt_t*)part->elts;
    nelts = part->nelts;

    req_payload->propagation_headers = njt_pcalloc(r->pool, nelts * sizeof(http_headers));
    req_payload->request_headers = njt_pcalloc(r->pool, nelts * sizeof(http_headers));
    int request_headers_idx = 0;
    int propagation_headers_idx = 0;
    for (njt_uint_t j = 0; j < nelts; j++) {

        h = &header[j];
        for (int i = 0; i < headers_len; i++) {

            if (strcmp(h->key.data, httpHeaders[i]) == 0) {
                req_payload->propagation_headers[propagation_headers_idx].name = httpHeaders[i];
                req_payload->propagation_headers[propagation_headers_idx].value = (const char*)(h->value).data;
                if (req_payload->propagation_headers[propagation_headers_idx].value == NULL) {
                    req_payload->propagation_headers[propagation_headers_idx].value = "";
                }
                propagation_headers_idx++;
                break;
            }
        }

        req_payload->request_headers[request_headers_idx].name = (const char*)(h->key).data;
        req_payload->request_headers[request_headers_idx].value = (const char*)(h->value).data;
        if (req_payload->request_headers[request_headers_idx].value == NULL) {
            req_payload->request_headers[request_headers_idx].value = "";
        }
        request_headers_idx++;
    }
    req_payload->propagation_count = propagation_headers_idx;
    req_payload->request_headers_count = request_headers_idx;
}

static void fillResponsePayload(response_payload* res_payload, njt_http_request_t* r)
{
    if (!r->pool) {
        return;
    }

    njt_list_part_t  *part;
    njt_table_elt_t  *header;
    njt_uint_t       nelts;
    njt_table_elt_t  *h;

    part = &r->headers_out.headers.part;
    header = (njt_table_elt_t*)part->elts;
    nelts = part->nelts;

    res_payload->response_headers = njt_pcalloc(r->pool, nelts * sizeof(http_headers));
    njt_uint_t headers_count = 0;

    for (njt_uint_t j = 0; j < nelts; j++) {
        h = &header[j];

        if (headers_count < nelts) {
            res_payload->response_headers[headers_count].name = (const char*)(h->key).data;
            res_payload->response_headers[headers_count].value = (const char*)(h->value).data;
            if (res_payload->response_headers[headers_count].value == NULL) {
                res_payload->response_headers[headers_count].value = "";
            }
            headers_count++;
        }
    }
    res_payload->response_headers_count = headers_count;
}
