
/*
 * !!! DO NOT EDIT DIRECTLY !!!
 * This file was automatically generated from the following template:
 *
 * src/subsys/njt_subsys_lua_semaphore.h.tt2
 */


/*
 * Copyright (C) Yichun Zhang (agentzh)
 * Copyright (C) 2021-2023  TMLake(Beijing) Technology Co., Ltd.
 * Copyright (C) cuiweixie
 * I hereby assign copyright in this code to the lua-njet-module project,
 * to be licensed under the same terms as the rest of the code.
 */


#ifndef _NJT_STREAM_LUA_SEMAPHORE_H_INCLUDED_
#define _NJT_STREAM_LUA_SEMAPHORE_H_INCLUDED_


#include "njt_stream_lua_common.h"


typedef struct njt_stream_lua_sema_mm_block_s {
    njt_uint_t                               used;
    njt_stream_lua_sema_mm_t                *mm;
    njt_uint_t                               epoch;
} njt_stream_lua_sema_mm_block_t;


struct njt_stream_lua_sema_mm_s {
    njt_queue_t                          free_queue;
    njt_uint_t                           total;
    njt_uint_t                           used;
    njt_uint_t                           num_per_block;
    njt_uint_t                           cur_epoch;
    njt_stream_lua_main_conf_t          *lmcf;
};


typedef struct njt_stream_lua_sema_s {
    njt_queue_t                                  wait_queue;
    njt_queue_t                                  chain;
    njt_event_t                                  sem_event;
    njt_stream_lua_sema_mm_block_t              *block;
    int                                          resource_count;
    unsigned                                     wait_count;
} njt_stream_lua_sema_t;


void njt_stream_lua_sema_mm_cleanup(void *data);
njt_int_t njt_stream_lua_sema_mm_init(njt_conf_t *cf,
    njt_stream_lua_main_conf_t *lmcf);


#endif /* _NJT_STREAM_LUA_SEMAPHORE_H_INCLUDED_ */

/* vi:set ft=c ts=4 sw=4 et fdm=marker: */