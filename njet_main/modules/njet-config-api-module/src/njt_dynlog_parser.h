

/* This file was generated by JSON Schema to C.
 * Any changes made to it will be lost on regeneration. 

 * Copyright (C) 2021-2023  TMLake(Beijing) Technology Co., Ltd.
 */

#ifndef PARSER_DYN_LOG_H
#define PARSER_DYN_LOG_H
#include <stdint.h>
#include <stdbool.h>
#include "njt_core.h"
#include "js2c_njet_builtins.h"
/* ===================== Generated type declarations ===================== */
typedef struct dynlog_accessLog_t_s dynlog_accessLog_t; //forward decl for public definition
typedef njt_str_t dynlog_accessLog_path_t;

typedef njt_str_t dynlog_accessLog_formatName_t;

typedef struct dynlog_accessLog_t_s {
    dynlog_accessLog_path_t path;
    dynlog_accessLog_formatName_t formatName;
    unsigned int is_path_set:1;
    unsigned int is_formatName_set:1;
} dynlog_accessLog_t;

dynlog_accessLog_path_t* get_dynlog_accessLog_path(dynlog_accessLog_t *out);
dynlog_accessLog_formatName_t* get_dynlog_accessLog_formatName(dynlog_accessLog_t *out);
void set_dynlog_accessLog_path(dynlog_accessLog_t* obj, dynlog_accessLog_path_t* field);
void set_dynlog_accessLog_formatName(dynlog_accessLog_t* obj, dynlog_accessLog_formatName_t* field);
dynlog_accessLog_t* create_dynlog_accessLog(njt_pool_t *pool);
typedef struct dynlog_locationDef_t_s dynlog_locationDef_t; //forward decl for public definition
typedef njt_str_t dynlog_locationDef_location_t;

typedef dynlog_locationDef_t dynlog_locationDef_locations_item_t; //ref def
typedef njt_array_t  dynlog_locationDef_locations_t;
typedef bool dynlog_locationDef_accessLogOn_t;
typedef dynlog_accessLog_t dynlog_locationDef_accessLogs_item_t; //ref def
typedef njt_array_t  dynlog_locationDef_accessLogs_t;
typedef struct dynlog_locationDef_t_s {
    dynlog_locationDef_location_t location;
    dynlog_locationDef_locations_t *locations;
    dynlog_locationDef_accessLogOn_t accessLogOn;
    dynlog_locationDef_accessLogs_t *accessLogs;
    unsigned int is_location_set:1;
    unsigned int is_locations_set:1;
    unsigned int is_accessLogOn_set:1;
    unsigned int is_accessLogs_set:1;
} dynlog_locationDef_t;

dynlog_locationDef_locations_item_t* get_dynlog_locationDef_locations_item(dynlog_locationDef_locations_t *out, size_t idx);
dynlog_locationDef_accessLogs_item_t* get_dynlog_locationDef_accessLogs_item(dynlog_locationDef_accessLogs_t *out, size_t idx);
dynlog_locationDef_location_t* get_dynlog_locationDef_location(dynlog_locationDef_t *out);
// CHECK ARRAY not exceeding bounds before calling this func
dynlog_locationDef_locations_t* get_dynlog_locationDef_locations(dynlog_locationDef_t *out);
dynlog_locationDef_accessLogOn_t get_dynlog_locationDef_accessLogOn(dynlog_locationDef_t *out);
// CHECK ARRAY not exceeding bounds before calling this func
dynlog_locationDef_accessLogs_t* get_dynlog_locationDef_accessLogs(dynlog_locationDef_t *out);
void set_dynlog_locationDef_location(dynlog_locationDef_t* obj, dynlog_locationDef_location_t* field);
int add_item_dynlog_locationDef_locations(dynlog_locationDef_locations_t *src, dynlog_locationDef_locations_item_t* items);
dynlog_locationDef_locations_t* create_dynlog_locationDef_locations(njt_pool_t *pool, size_t nelts);
void set_dynlog_locationDef_locations(dynlog_locationDef_t* obj, dynlog_locationDef_locations_t* field);
void set_dynlog_locationDef_accessLogOn(dynlog_locationDef_t* obj, dynlog_locationDef_accessLogOn_t field);
int add_item_dynlog_locationDef_accessLogs(dynlog_locationDef_accessLogs_t *src, dynlog_locationDef_accessLogs_item_t* items);
dynlog_locationDef_accessLogs_t* create_dynlog_locationDef_accessLogs(njt_pool_t *pool, size_t nelts);
void set_dynlog_locationDef_accessLogs(dynlog_locationDef_t* obj, dynlog_locationDef_accessLogs_t* field);
dynlog_locationDef_t* create_dynlog_locationDef(njt_pool_t *pool);
typedef struct dynlog_accessLogFormat_t_s dynlog_accessLogFormat_t; //forward decl for public definition
typedef njt_str_t dynlog_accessLogFormat_name_t;

typedef enum dynlog_accessLogFormat_escape_t_e{
    DYNLOG_ACCESSLOGFORMAT_ESCAPE_DEFAULT,
    DYNLOG_ACCESSLOGFORMAT_ESCAPE_JSON,
    DYNLOG_ACCESSLOGFORMAT_ESCAPE_NONE
} dynlog_accessLogFormat_escape_t;

typedef njt_str_t dynlog_accessLogFormat_format_t;

typedef struct dynlog_accessLogFormat_t_s {
    dynlog_accessLogFormat_name_t name;
    dynlog_accessLogFormat_escape_t escape;
    dynlog_accessLogFormat_format_t format;
    unsigned int is_name_set:1;
    unsigned int is_escape_set:1;
    unsigned int is_format_set:1;
} dynlog_accessLogFormat_t;

dynlog_accessLogFormat_name_t* get_dynlog_accessLogFormat_name(dynlog_accessLogFormat_t *out);
dynlog_accessLogFormat_escape_t get_dynlog_accessLogFormat_escape(dynlog_accessLogFormat_t *out);
dynlog_accessLogFormat_format_t* get_dynlog_accessLogFormat_format(dynlog_accessLogFormat_t *out);
void set_dynlog_accessLogFormat_name(dynlog_accessLogFormat_t* obj, dynlog_accessLogFormat_name_t* field);
void set_dynlog_accessLogFormat_escape(dynlog_accessLogFormat_t* obj, dynlog_accessLogFormat_escape_t field);
void set_dynlog_accessLogFormat_format(dynlog_accessLogFormat_t* obj, dynlog_accessLogFormat_format_t* field);
dynlog_accessLogFormat_t* create_dynlog_accessLogFormat(njt_pool_t *pool);
typedef njt_str_t dynlog_servers_item_listens_item_t;

typedef njt_array_t  dynlog_servers_item_listens_t;
typedef njt_str_t dynlog_servers_item_serverNames_item_t;

typedef njt_array_t  dynlog_servers_item_serverNames_t;
typedef dynlog_locationDef_t dynlog_servers_item_locations_item_t; //ref def
typedef njt_array_t  dynlog_servers_item_locations_t;
typedef struct dynlog_servers_item_t_s {
    dynlog_servers_item_listens_t *listens;
    dynlog_servers_item_serverNames_t *serverNames;
    dynlog_servers_item_locations_t *locations;
    unsigned int is_listens_set:1;
    unsigned int is_serverNames_set:1;
    unsigned int is_locations_set:1;
} dynlog_servers_item_t;

typedef njt_array_t  dynlog_servers_t;
typedef dynlog_accessLogFormat_t dynlog_accessLogFormats_item_t; //ref def
typedef njt_array_t  dynlog_accessLogFormats_t;
typedef struct dynlog_t_s {
    dynlog_servers_t *servers;
    dynlog_accessLogFormats_t *accessLogFormats;
    unsigned int is_servers_set:1;
    unsigned int is_accessLogFormats_set:1;
} dynlog_t;

dynlog_servers_item_listens_item_t* get_dynlog_servers_item_listens_item(dynlog_servers_item_listens_t *out, size_t idx);
dynlog_servers_item_serverNames_item_t* get_dynlog_servers_item_serverNames_item(dynlog_servers_item_serverNames_t *out, size_t idx);
dynlog_servers_item_locations_item_t* get_dynlog_servers_item_locations_item(dynlog_servers_item_locations_t *out, size_t idx);
// CHECK ARRAY not exceeding bounds before calling this func
dynlog_servers_item_listens_t* get_dynlog_servers_item_listens(dynlog_servers_item_t *out);
// CHECK ARRAY not exceeding bounds before calling this func
dynlog_servers_item_serverNames_t* get_dynlog_servers_item_serverNames(dynlog_servers_item_t *out);
// CHECK ARRAY not exceeding bounds before calling this func
dynlog_servers_item_locations_t* get_dynlog_servers_item_locations(dynlog_servers_item_t *out);
dynlog_servers_item_t* get_dynlog_servers_item(dynlog_servers_t *out, size_t idx);
dynlog_accessLogFormats_item_t* get_dynlog_accessLogFormats_item(dynlog_accessLogFormats_t *out, size_t idx);
// CHECK ARRAY not exceeding bounds before calling this func
dynlog_servers_t* get_dynlog_servers(dynlog_t *out);
// CHECK ARRAY not exceeding bounds before calling this func
dynlog_accessLogFormats_t* get_dynlog_accessLogFormats(dynlog_t *out);
int add_item_dynlog_servers_item_listens(dynlog_servers_item_listens_t *src, dynlog_servers_item_listens_item_t* items);
dynlog_servers_item_listens_t* create_dynlog_servers_item_listens(njt_pool_t *pool, size_t nelts);
void set_dynlog_servers_item_listens(dynlog_servers_item_t* obj, dynlog_servers_item_listens_t* field);
int add_item_dynlog_servers_item_serverNames(dynlog_servers_item_serverNames_t *src, dynlog_servers_item_serverNames_item_t* items);
dynlog_servers_item_serverNames_t* create_dynlog_servers_item_serverNames(njt_pool_t *pool, size_t nelts);
void set_dynlog_servers_item_serverNames(dynlog_servers_item_t* obj, dynlog_servers_item_serverNames_t* field);
int add_item_dynlog_servers_item_locations(dynlog_servers_item_locations_t *src, dynlog_servers_item_locations_item_t* items);
dynlog_servers_item_locations_t* create_dynlog_servers_item_locations(njt_pool_t *pool, size_t nelts);
void set_dynlog_servers_item_locations(dynlog_servers_item_t* obj, dynlog_servers_item_locations_t* field);
dynlog_servers_item_t* create_dynlog_servers_item(njt_pool_t *pool);
int add_item_dynlog_servers(dynlog_servers_t *src, dynlog_servers_item_t* items);
dynlog_servers_t* create_dynlog_servers(njt_pool_t *pool, size_t nelts);
void set_dynlog_servers(dynlog_t* obj, dynlog_servers_t* field);
int add_item_dynlog_accessLogFormats(dynlog_accessLogFormats_t *src, dynlog_accessLogFormats_item_t* items);
dynlog_accessLogFormats_t* create_dynlog_accessLogFormats(njt_pool_t *pool, size_t nelts);
void set_dynlog_accessLogFormats(dynlog_t* obj, dynlog_accessLogFormats_t* field);
dynlog_t* create_dynlog(njt_pool_t *pool);
dynlog_t* json_parse_dynlog(njt_pool_t *pool, const njt_str_t *json_string, js2c_parse_error_t *err_ret);
njt_str_t* to_json_dynlog(njt_pool_t *pool, dynlog_t *out, njt_int_t flags);
#endif /* PARSER_DYN_LOG_H */