/*************************************************************************************
 Copyright (C) 2021-2023  TMLake(Beijing) Technology Co., Ltd.
 File name    : njt_agent_dyn_telemetry_module.c
 Version      : 1.0
 Author       : ChenLubo
 Date         : 2023/2/23/020 
 Description  : 
 Other        :
 History      :
 <author>       <time>          <version >      <desc>
 ChenLubo        2023/2/23/020       1.1             
***********************************************************************************/
//
// Created by Administrator on 2023/2/23/020.
//
extern "C" {
#include <njt_core.h>
#include <njt_http_kv_module.h>
#include <njt_http.h>
#include <njt_json_util.h>
#include <njt_http_util.h>
#include "njt_dyn_telemetry_parser.h"
#include <njt_rpc_result_util.h>

extern njt_module_t njt_http_core_module;
}

#include "location_config.h"


static njt_int_t njt_dyn_telemetry_update_locs_telemetry(njt_array_t *locs,njt_queue_t *q, njt_rpc_result_t *rpc_result){
    njt_http_core_loc_conf_t            *clcf;
    njt_http_location_queue_t           *hlq;
    dyn_telemetry_servers_item_locations_item_t            *dtil;
    u_char                               data_buf[1024];
    u_char                              *end;
    njt_uint_t                           j;
    njt_queue_t                         *tq;
    OtelNjtLocationConf                 *llcf;
    njt_str_t                            parent_conf_path;
    njt_str_t                            rpc_data_str;
    bool                                 found = false;
    njt_str_t                           *name;

    rpc_data_str.data = data_buf;
    rpc_data_str.len = 0;

    if(q == NULL){
        return NJT_OK;
    }

    if(rpc_result){
        parent_conf_path = rpc_result->conf_path;
    }

    for( j = 0; j < locs->nelts ; ++j ){
        dtil = get_dyn_telemetry_servers_item_locations_item(locs, j);
        if(dtil == NULL || !dtil->is_location_set){
            end = njt_snprintf(data_buf, sizeof(data_buf) - 1, " index %d not set location name", j);
            rpc_data_str.len = end - data_buf;
            njt_rpc_result_add_error_data(rpc_result, &rpc_data_str);
            continue;
        }

        name = get_dyn_telemetry_locationDef_location(dtil);

        tq = njt_queue_head(q);
        found = false;
        for (;tq!= njt_queue_sentinel(q);tq = njt_queue_next(tq)) {
            hlq = njt_queue_data(tq, njt_http_location_queue_t, queue);
            clcf = hlq->exact == NULL ? hlq->inclusive : hlq->exact;
            if (clcf != NULL && njt_http_location_full_name_cmp(clcf->full_name, *name) == 0) {
                if(rpc_result){
                    njt_rpc_result_set_conf_path(rpc_result, &parent_conf_path);
                    end = njt_snprintf(data_buf,sizeof(data_buf) - 1,".locations[%V]", &clcf->full_name);
                    rpc_data_str.len = end - data_buf;

                    njt_rpc_result_append_conf_path(rpc_result, &rpc_data_str);
                }
                
                llcf = (OtelNjtLocationConf*)njt_http_get_module_loc_conf(clcf, njt_otel_module);
                if(llcf == NULL){
                    continue;
                }

                if(dtil->is_opentelemetry_set){
                    llcf->enabled = dtil->opentelemetry;
                    njt_log_error(NJT_LOG_INFO, njt_cycle->log, 0, "change location %V telemetry to %i", &dtil->location, dtil->opentelemetry);
                }
                
                found = true;
                if (dtil->is_locations_set && dtil->locations && dtil->locations->nelts > 0) {
                    njt_dyn_telemetry_update_locs_telemetry(dtil->locations, clcf->old_locations, rpc_result);
                }
            }
        }

        if(!found){
            end = njt_snprintf(data_buf,sizeof(data_buf) - 1," location[%V] not found", name);
            rpc_data_str.len = end - data_buf;
            njt_rpc_result_add_error_data(rpc_result, &rpc_data_str);
        }
    }
    return NJT_OK;
}

static njt_int_t njt_dyn_telemetry_update_telemetry(njt_pool_t *pool,dyn_telemetry_t *api_data,
                                njt_rpc_result_t *rpc_result){

    njt_cycle_t                        *cycle,*new_cycle;
    njt_http_core_srv_conf_t           *cscf;
    njt_http_core_loc_conf_t           *clcf;
    dyn_telemetry_servers_item_t       *dsi;
    njt_uint_t                          i;
    njt_str_t                           *port, *serverName;
    u_char                              data_buf[1024];
    u_char                             *end;
    njt_str_t                           rpc_data_str;
    njt_int_t                           rc = NJT_OK;

    rpc_data_str.data = data_buf;
    rpc_data_str.len = 0;

    cycle = (njt_cycle_t*)njt_cycle;

    if(api_data->is_servers_set && api_data->servers != NULL){
        for(i = 0; i < api_data->servers->nelts; ++i){
            dsi = get_dyn_telemetry_servers_item(api_data->servers, i);
            if (dsi == NULL || !dsi->is_listens_set || !dsi->is_serverNames_set 
                    || dsi->listens->nelts < 1 
                    || dsi->serverNames->nelts < 1) {
                // listens or server_names is empty
                end = njt_snprintf(data_buf, sizeof(data_buf) - 1, 
                    " server parameters error, listens or serverNames or locations is empty,at position %d", i);
                rpc_data_str.len = end - data_buf;
                njt_rpc_result_add_error_data(rpc_result, &rpc_data_str);
                continue;
            }

            port = get_dyn_telemetry_servers_item_listens_item(dsi->listens, 0);
            serverName = get_dyn_telemetry_servers_item_serverNames_item(dsi->serverNames, 0);
            njt_str_null(&rpc_result->conf_path);

            cscf = njt_http_get_srv_by_port(cycle, port, serverName);
            if (cscf == NULL)
            {
                njt_log_error(NJT_LOG_INFO, pool->log, 0, "can`t find server by listen:%V server_name:%V ",
                            port, serverName);
                end = njt_snprintf(data_buf, sizeof(data_buf) - 1, 
                    " can`t find server by listen[%V] server_name[%V]", port, serverName);
                rpc_data_str.len = end - data_buf;
                njt_rpc_result_add_error_data(rpc_result, &rpc_data_str);
                continue;
            }

            njt_log_error(NJT_LOG_INFO, pool->log, 0, "dyntelemetry start update listen:%V server_name:%V",
                    port, serverName);

            end = njt_snprintf(data_buf, sizeof(data_buf) - 1, " listen[%V] server_name[%V]", port, serverName);
            rpc_data_str.len = end - data_buf;
            njt_rpc_result_set_conf_path(rpc_result, &rpc_data_str);

            clcf = (njt_http_core_loc_conf_t*)njt_http_get_module_loc_conf(cscf->ctx, njt_http_core_module);
            if(clcf == NULL){
                njt_log_error(NJT_LOG_INFO, pool->log, 0, " dyntelemetry can`t find location config by listen:%V server_name:%V ",
                            port, serverName);
                end = njt_snprintf(data_buf, sizeof(data_buf) - 1, " can`t find location config by listen[%V] server_name[%V]", port, serverName);
                rpc_data_str.len = end - data_buf;
                njt_rpc_result_add_error_data(rpc_result, &rpc_data_str);
                continue;
            }

            if(dsi->is_locations_set && dsi->locations->nelts > 0){
                rc = njt_dyn_telemetry_update_locs_telemetry(dsi->locations, clcf->old_locations, rpc_result);
                if(rc != NJT_OK){
                    njt_log_error(NJT_LOG_INFO, pool->log, 0, "update telemetry error, listen:%V server_name:%V",
                        port, serverName);
                }
            }
        }
    }
    return NJT_OK;
}



static void njt_dyn_telemetry_dump_locs_json(njt_pool_t *pool,
            njt_queue_t *locations, dyn_telemetry_servers_item_locations_t *loc_items){
    njt_http_core_loc_conf_t                    *clcf;
    njt_http_location_queue_t                   *hlq;
    njt_queue_t                                 *q,*tq;
    OtelNjtLocationConf                         *llcf;
    dyn_telemetry_servers_item_locations_item_t *loc_item;

    if(locations == NULL){
        return;
    }

    q = locations;
    if(njt_queue_empty(q)){
        return;
    }

    tq = njt_queue_head(q);
    for (;tq!= njt_queue_sentinel(q);tq = njt_queue_next(tq)){
        hlq = njt_queue_data(tq,njt_http_location_queue_t,queue);
        clcf = hlq->exact == NULL ? hlq->inclusive : hlq->exact;
        if(clcf == NULL){
            continue;
        }
        llcf = (OtelNjtLocationConf*)njt_http_get_module_loc_conf(clcf,njt_otel_module);
        if(llcf == NULL){
            continue;
        }

        loc_item = create_dyn_telemetry_locationDef(pool);
        if(loc_item == NULL){
            continue;
        }
        set_dyn_telemetry_locationDef_location(loc_item, &clcf->full_name);
        set_dyn_telemetry_locationDef_opentelemetry(loc_item, llcf->enabled);

        if (clcf->old_locations) {
            set_dyn_telemetry_locationDef_locations(loc_item, create_dyn_telemetry_locationDef_locations(pool, 4));
            if(loc_item->locations != NULL){
                njt_dyn_telemetry_dump_locs_json(pool, clcf->old_locations, loc_item->locations);
            }
        }

        add_item_dyn_telemetry_servers_item_locations(loc_items, loc_item);
    }
}

njt_str_t dyn_telemetry_update_srv_err_msg=njt_string("{\"code\":500,\"msg\":\"server error\"}");


static njt_str_t *njt_dyn_telemetry_dump_telemetry_conf(njt_cycle_t *cycle,njt_pool_t *pool){
    njt_http_core_loc_conf_t        *clcf;
    njt_http_core_main_conf_t       *hcmcf;
    njt_http_core_srv_conf_t        **cscfp;
    njt_uint_t                      i,j;
    njt_array_t                     *array;
    njt_str_t                       *tmp_str;
    njt_http_server_name_t          *server_name;
    dyn_telemetry_t                 dynjson_obj;
    dyn_telemetry_servers_item_t    *server_item;

    hcmcf = (njt_http_core_main_conf_t*)njt_http_cycle_get_module_main_conf(cycle,njt_http_core_module);
    if(hcmcf == NULL){
        goto err;
    }

    set_dyn_telemetry_servers(&dynjson_obj, create_dyn_telemetry_servers(pool, 4));
    if(dynjson_obj.servers == NULL){
        goto err;
    }

    cscfp = (njt_http_core_srv_conf_t**)hcmcf->servers.elts;
    for( i = 0; i < hcmcf->servers.nelts; i++){
        server_item = create_dyn_telemetry_servers_item(pool);
        if(server_item == NULL){
            goto err;
        }

        set_dyn_telemetry_servers_item_listens(server_item, create_dyn_telemetry_servers_item_listens(pool, 4));
        set_dyn_telemetry_servers_item_serverNames(server_item, create_dyn_telemetry_servers_item_serverNames(pool, 4));
        set_dyn_telemetry_servers_item_locations(server_item, create_dyn_telemetry_servers_item_locations(pool, 4));

        array = njt_array_create(pool, 4, sizeof(njt_str_t));
        if(array == NULL){
            goto err;
        }
        njt_http_get_listens_by_server(array, cscfp[i]);

        for (j = 0; j < array->nelts; ++j) {
            tmp_str = (njt_str_t *)(array->elts)+ j;
            add_item_dyn_telemetry_servers_item_listens(server_item->listens, tmp_str);
        }

        server_name = (njt_http_server_name_t*)cscfp[i]->server_names.elts;
        for (j = 0; j < cscfp[i]->server_names.nelts; ++j) {
            tmp_str = &server_name[j].name;
            add_item_dyn_telemetry_servers_item_serverNames(server_item->serverNames,tmp_str);
        }

        clcf = (njt_http_core_loc_conf_t*)njt_http_get_module_loc_conf(cscfp[i]->ctx,njt_http_core_module);
        if(clcf != NULL){
            njt_dyn_telemetry_dump_locs_json(pool,clcf->old_locations, server_item->locations);
        }

        add_item_dyn_telemetry_servers(dynjson_obj.servers, server_item);
    }

    return to_json_dyn_telemetry(pool, &dynjson_obj, OMIT_NULL_ARRAY | OMIT_NULL_OBJ | OMIT_NULL_STR);

err:
    return &dyn_telemetry_update_srv_err_msg;
}

static u_char* njt_agent_dyn_telemetry_rpc_handler(njt_str_t *topic, njt_str_t *request, int* len, void *data){
    njt_cycle_t     *cycle;
    njt_str_t       *msg;
    u_char          *buf;

    buf = NULL;
    cycle = (njt_cycle_t*) njt_cycle;
    *len = 0 ;
    njt_pool_t *pool = NULL;
    pool = njt_create_pool(njt_pagesize,njt_cycle->log);
    if(pool == NULL){
        njt_log_error(NJT_LOG_EMERG, pool->log, 0, "njt_agent_dyn_telemetry_change_handler create pool error");
        goto end;
    }
    msg = njt_dyn_telemetry_dump_telemetry_conf(cycle,pool);
    buf = (u_char*)njt_calloc(msg->len,cycle->log);
    if(buf == NULL){
        goto end;
    }
    njt_log_error(NJT_LOG_INFO, pool->log, 0, "send json : %V", msg);
    njt_memcpy(buf, msg->data, msg->len);
    *len = msg->len;

    end:
    if(pool != NULL){
        njt_destroy_pool(pool);
    }

    return buf;
}


static int  njt_agent_dyn_telemetry_update_handler(njt_str_t *key, njt_str_t *value, void *data, njt_str_t *out_msg){
    njt_int_t                           rc = NJT_OK;
    dyn_telemetry_t                     *api_data = NULL;
    njt_pool_t                         *pool = NULL;
    njt_rpc_result_t                   *rpc_result = NULL;
    js2c_parse_error_t                 err_info;

    rpc_result = njt_rpc_result_create();
    if(!rpc_result){
        if(out_msg != NULL){
            njt_rpc_result_set_code(rpc_result, NJT_RPC_RSP_ERR);
            njt_rpc_result_set_msg(rpc_result, (u_char *)" create rpc_result error");
        }
        rc = NJT_ERROR;

        goto end;
    }

    if(value->len < 2 ){
        njt_rpc_result_set_code(rpc_result, NJT_RPC_RSP_ERR_INPUT_PARAM);
        njt_rpc_result_set_msg(rpc_result, (u_char *)" input param not valid, less then 2 byte");
        rc = NJT_ERROR;

        goto end;
    }

    pool = njt_create_pool(njt_pagesize, njt_cycle->log);
    if(pool == NULL){
        njt_log_error(NJT_LOG_EMERG, pool->log, 0, "njt_agent_dyn_telemetry_change_handler create pool error");
        njt_rpc_result_set_code(rpc_result, NJT_RPC_RSP_ERR_MEM_ALLOC);
        njt_rpc_result_set_msg(rpc_result, (u_char *)" update handler create pool error");
        rc = NJT_ERROR;

        goto end;
    }


    api_data = json_parse_dyn_telemetry(pool, value, &err_info);
    if (api_data == NULL)
    {
        njt_log_error(NJT_LOG_ERR, pool->log, 0, 
                "json_parse_dyn_telemetry err: %V",  &err_info.err_str);
        njt_rpc_result_set_code(rpc_result, NJT_RPC_RSP_ERR_JSON);
        njt_rpc_result_set_msg2(rpc_result, &err_info.err_str);

        rc = NJT_ERROR;
        goto end;
    }

    njt_rpc_result_set_code(rpc_result,NJT_RPC_RSP_SUCCESS);

    rc = njt_dyn_telemetry_update_telemetry(pool, api_data, rpc_result);
    if(rc != NJT_OK){
        njt_rpc_result_set_code(rpc_result,NJT_RPC_RSP_ERR);
        njt_rpc_result_set_msg(rpc_result, (u_char *)" telemetry update fail");
    }else{
        if(rpc_result->data != NULL && rpc_result->data->nelts > 0){
            njt_rpc_result_set_code(rpc_result, NJT_RPC_RSP_PARTIAL_SUCCESS);
        }
    }

    end:
    if (rc != NJT_OK) {
        njt_str_t msg=njt_string("");
        njt_kv_sendmsg(key,&msg, 0);
    }

    if(out_msg){
        njt_rpc_result_to_json_str(rpc_result,out_msg);
    }

    if(pool != NULL){
        njt_destroy_pool(pool);
    }

    if(rpc_result){
        njt_rpc_result_destroy(rpc_result);
    }

    return rc;
}


static int  njt_agent_dyn_telemetry_change_handler(njt_str_t *key, njt_str_t *value, void *data){
    return njt_agent_dyn_telemetry_update_handler(key, value, data, NULL);
}

static u_char* njt_agent_dyn_telemetry_put_handler(njt_str_t *topic, njt_str_t *request, int* len, void *data){
    njt_str_t err_json_msg;
    njt_str_null(&err_json_msg);
    njt_agent_dyn_telemetry_update_handler(topic, request, data, &err_json_msg);
    *len = err_json_msg.len;
    return err_json_msg.data;
}

static njt_int_t njt_agent_dyn_telemetry_init_process(njt_cycle_t* cycle){
    // njt_str_t  rpc_key = njt_string("otel");
    // njt_reg_kv_change_handler(&rpc_key, njt_agent_dyn_telemetry_change_handler,njt_agent_dyn_telemetry_rpc_handler, NULL);
    // njt_reg_kv_msg_handler(&rpc_key, njt_agent_dyn_telemetry_change_handler, njt_agent_dyn_telemetry_put_handler, njt_agent_dyn_telemetry_rpc_handler, NULL);
    
    njt_str_t otel_rpc_key = njt_string("otel");
    njt_kv_reg_handler_t h;
    njt_memzero(&h, sizeof(njt_kv_reg_handler_t));
    h.key = &otel_rpc_key;
    h.rpc_get_handler = njt_agent_dyn_telemetry_rpc_handler;
    h.rpc_put_handler = njt_agent_dyn_telemetry_put_handler;
    h.handler = njt_agent_dyn_telemetry_change_handler;
    h.api_type = NJT_KV_API_TYPE_DECLATIVE;
    njt_kv_reg_handler(&h);

    return NJT_OK;
}


static njt_http_module_t njt_agent_dyn_telemetry_module_ctx = {
        NULL,                                   /* preconfiguration */
        NULL,                                   /* postconfiguration */

        NULL,                                   /* create main configuration */
        NULL,                                  /* init main configuration */

        NULL,                                  /* create server configuration */
        NULL,                                  /* merge server configuration */

        NULL,                                   /* create location configuration */
        NULL                                    /* merge location configuration */
};

njt_module_t njt_agent_dyn_telemetry_module = {
        NJT_MODULE_V1,
        &njt_agent_dyn_telemetry_module_ctx,                 /* module context */
        NULL,                                   /* module directives */
        NJT_HTTP_MODULE,                        /* module type */
        NULL,                                   /* init master */
        NULL,                                   /* init module */
        njt_agent_dyn_telemetry_init_process,          /* init process */
        NULL,                                   /* init thread */
        NULL,                                   /* exit thread */
        NULL,                                   /* exit process */
        NULL,                                   /* exit master */
        NJT_MODULE_V1_PADDING
};
