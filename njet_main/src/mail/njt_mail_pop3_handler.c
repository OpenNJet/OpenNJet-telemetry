
/*
 * Copyright (C) Igor Sysoev
 * Copyright (C) Nginx, Inc.
 * Copyright (C) 2021-2023  TMLake(Beijing) Technology Co., Ltd.
 */


#include <njt_config.h>
#include <njt_core.h>
#include <njt_event.h>
#include <njt_mail.h>
#include <njt_mail_pop3_module.h>


static njt_int_t njt_mail_pop3_user(njt_mail_session_t *s, njt_connection_t *c);
static njt_int_t njt_mail_pop3_pass(njt_mail_session_t *s, njt_connection_t *c);
static njt_int_t njt_mail_pop3_capa(njt_mail_session_t *s, njt_connection_t *c,
    njt_int_t stls);
static njt_int_t njt_mail_pop3_stls(njt_mail_session_t *s, njt_connection_t *c);
static njt_int_t njt_mail_pop3_apop(njt_mail_session_t *s, njt_connection_t *c);
static njt_int_t njt_mail_pop3_auth(njt_mail_session_t *s, njt_connection_t *c);


static u_char  pop3_greeting[] = "+OK POP3 ready" CRLF;
static u_char  pop3_ok[] = "+OK" CRLF;
static u_char  pop3_next[] = "+ " CRLF;
static u_char  pop3_username[] = "+ VXNlcm5hbWU6" CRLF;
static u_char  pop3_password[] = "+ UGFzc3dvcmQ6" CRLF;
static u_char  pop3_invalid_command[] = "-ERR invalid command" CRLF;


void
njt_mail_pop3_init_session(njt_mail_session_t *s, njt_connection_t *c)
{
    u_char                    *p;
    njt_mail_core_srv_conf_t  *cscf;
    njt_mail_pop3_srv_conf_t  *pscf;

    pscf = njt_mail_get_module_srv_conf(s, njt_mail_pop3_module);
    cscf = njt_mail_get_module_srv_conf(s, njt_mail_core_module);

    if (pscf->auth_methods
        & (NJT_MAIL_AUTH_APOP_ENABLED|NJT_MAIL_AUTH_CRAM_MD5_ENABLED))
    {
        if (njt_mail_salt(s, c, cscf) != NJT_OK) {
            njt_mail_session_internal_server_error(s);
            return;
        }

        s->out.data = njt_pnalloc(c->pool, sizeof(pop3_greeting) + s->salt.len);
        if (s->out.data == NULL) {
            njt_mail_session_internal_server_error(s);
            return;
        }

        p = njt_cpymem(s->out.data, pop3_greeting, sizeof(pop3_greeting) - 3);
        *p++ = ' ';
        p = njt_cpymem(p, s->salt.data, s->salt.len);

        s->out.len = p - s->out.data;

    } else {
        njt_str_set(&s->out, pop3_greeting);
    }

    c->read->handler = njt_mail_pop3_init_protocol;

    njt_add_timer(c->read, cscf->timeout);

    if (njt_handle_read_event(c->read, 0) != NJT_OK) {
        njt_mail_close_connection(c);
    }

    njt_mail_send(c->write);
}


void
njt_mail_pop3_init_protocol(njt_event_t *rev)
{
    njt_connection_t    *c;
    njt_mail_session_t  *s;

    c = rev->data;

    c->log->action = "in auth state";

    if (rev->timedout) {
        njt_log_error(NJT_LOG_INFO, c->log, NJT_ETIMEDOUT, "client timed out");
        c->timedout = 1;
        njt_mail_close_connection(c);
        return;
    }

    s = c->data;

    if (s->buffer == NULL) {
        if (njt_array_init(&s->args, c->pool, 2, sizeof(njt_str_t))
            == NJT_ERROR)
        {
            njt_mail_session_internal_server_error(s);
            return;
        }

        s->buffer = njt_create_temp_buf(c->pool, 128);
        if (s->buffer == NULL) {
            njt_mail_session_internal_server_error(s);
            return;
        }
    }

    s->mail_state = njt_pop3_start;
    c->read->handler = njt_mail_pop3_auth_state;

    njt_mail_pop3_auth_state(rev);
}


void
njt_mail_pop3_auth_state(njt_event_t *rev)
{
    njt_int_t            rc;
    njt_connection_t    *c;
    njt_mail_session_t  *s;

    c = rev->data;
    s = c->data;

    njt_log_debug0(NJT_LOG_DEBUG_MAIL, c->log, 0, "pop3 auth state");

    if (rev->timedout) {
        njt_log_error(NJT_LOG_INFO, c->log, NJT_ETIMEDOUT, "client timed out");
        c->timedout = 1;
        njt_mail_close_connection(c);
        return;
    }

    if (s->out.len) {
        njt_log_debug0(NJT_LOG_DEBUG_MAIL, c->log, 0, "pop3 send handler busy");
        s->blocked = 1;

        if (njt_handle_read_event(c->read, 0) != NJT_OK) {
            njt_mail_close_connection(c);
            return;
        }

        return;
    }

    s->blocked = 0;

    rc = njt_mail_read_command(s, c);

    if (rc == NJT_AGAIN) {
        if (njt_handle_read_event(c->read, 0) != NJT_OK) {
            njt_mail_session_internal_server_error(s);
            return;
        }

        return;
    }

    if (rc == NJT_ERROR) {
        return;
    }

    njt_str_set(&s->out, pop3_ok);

    if (rc == NJT_OK) {
        switch (s->mail_state) {

        case njt_pop3_start:

            switch (s->command) {

            case NJT_POP3_USER:
                rc = njt_mail_pop3_user(s, c);
                break;

            case NJT_POP3_CAPA:
                rc = njt_mail_pop3_capa(s, c, 1);
                break;

            case NJT_POP3_APOP:
                rc = njt_mail_pop3_apop(s, c);
                break;

            case NJT_POP3_AUTH:
                rc = njt_mail_pop3_auth(s, c);
                break;

            case NJT_POP3_QUIT:
                s->quit = 1;
                break;

            case NJT_POP3_NOOP:
                break;

            case NJT_POP3_STLS:
                rc = njt_mail_pop3_stls(s, c);
                break;

            default:
                rc = NJT_MAIL_PARSE_INVALID_COMMAND;
                break;
            }

            break;

        case njt_pop3_user:

            switch (s->command) {

            case NJT_POP3_PASS:
                rc = njt_mail_pop3_pass(s, c);
                break;

            case NJT_POP3_CAPA:
                rc = njt_mail_pop3_capa(s, c, 0);
                break;

            case NJT_POP3_QUIT:
                s->quit = 1;
                break;

            case NJT_POP3_NOOP:
                break;

            default:
                rc = NJT_MAIL_PARSE_INVALID_COMMAND;
                break;
            }

            break;

        /* suppress warnings */
        case njt_pop3_passwd:
            break;

        case njt_pop3_auth_login_username:
            rc = njt_mail_auth_login_username(s, c, 0);

            njt_str_set(&s->out, pop3_password);
            s->mail_state = njt_pop3_auth_login_password;
            break;

        case njt_pop3_auth_login_password:
            rc = njt_mail_auth_login_password(s, c);
            break;

        case njt_pop3_auth_plain:
            rc = njt_mail_auth_plain(s, c, 0);
            break;

        case njt_pop3_auth_cram_md5:
            rc = njt_mail_auth_cram_md5(s, c);
            break;

        case njt_pop3_auth_external:
            rc = njt_mail_auth_external(s, c, 0);
            break;
        }
    }

    if (s->buffer->pos < s->buffer->last) {
        s->blocked = 1;
    }

    switch (rc) {

    case NJT_DONE:
        njt_mail_auth(s, c);
        return;

    case NJT_ERROR:
        njt_mail_session_internal_server_error(s);
        return;

    case NJT_MAIL_PARSE_INVALID_COMMAND:
        s->mail_state = njt_pop3_start;
        s->state = 0;

        njt_str_set(&s->out, pop3_invalid_command);

        /* fall through */

    case NJT_OK:

        s->args.nelts = 0;

        if (s->buffer->pos == s->buffer->last) {
            s->buffer->pos = s->buffer->start;
            s->buffer->last = s->buffer->start;
        }

        if (s->state) {
            s->arg_start = s->buffer->pos;
        }

        if (njt_handle_read_event(c->read, 0) != NJT_OK) {
            njt_mail_session_internal_server_error(s);
            return;
        }

        njt_mail_send(c->write);
    }
}

static njt_int_t
njt_mail_pop3_user(njt_mail_session_t *s, njt_connection_t *c)
{
    njt_str_t  *arg;

#if (NJT_MAIL_SSL)
    if (njt_mail_starttls_only(s, c)) {
        return NJT_MAIL_PARSE_INVALID_COMMAND;
    }
#endif

    if (s->args.nelts != 1) {
        return NJT_MAIL_PARSE_INVALID_COMMAND;
    }

    arg = s->args.elts;
    s->login.len = arg[0].len;
    s->login.data = njt_pnalloc(c->pool, s->login.len);
    if (s->login.data == NULL) {
        return NJT_ERROR;
    }

    njt_memcpy(s->login.data, arg[0].data, s->login.len);

    njt_log_debug1(NJT_LOG_DEBUG_MAIL, c->log, 0,
                   "pop3 login: \"%V\"", &s->login);

    s->mail_state = njt_pop3_user;

    return NJT_OK;
}


static njt_int_t
njt_mail_pop3_pass(njt_mail_session_t *s, njt_connection_t *c)
{
    njt_str_t  *arg;

    if (s->args.nelts != 1) {
        return NJT_MAIL_PARSE_INVALID_COMMAND;
    }

    arg = s->args.elts;
    s->passwd.len = arg[0].len;
    s->passwd.data = njt_pnalloc(c->pool, s->passwd.len);
    if (s->passwd.data == NULL) {
        return NJT_ERROR;
    }

    njt_memcpy(s->passwd.data, arg[0].data, s->passwd.len);

#if (NJT_DEBUG_MAIL_PASSWD)
    njt_log_debug1(NJT_LOG_DEBUG_MAIL, c->log, 0,
                   "pop3 passwd: \"%V\"", &s->passwd);
#endif

    return NJT_DONE;
}


static njt_int_t
njt_mail_pop3_capa(njt_mail_session_t *s, njt_connection_t *c, njt_int_t stls)
{
    njt_mail_pop3_srv_conf_t  *pscf;

    pscf = njt_mail_get_module_srv_conf(s, njt_mail_pop3_module);

#if (NJT_MAIL_SSL)

    if (stls && c->ssl == NULL) {
        njt_mail_ssl_conf_t  *sslcf;

        sslcf = njt_mail_get_module_srv_conf(s, njt_mail_ssl_module);

        if (sslcf->starttls == NJT_MAIL_STARTTLS_ON) {
            s->out = pscf->starttls_capability;
            return NJT_OK;
        }

        if (sslcf->starttls == NJT_MAIL_STARTTLS_ONLY) {
            s->out = pscf->starttls_only_capability;
            return NJT_OK;
        }
    }

#endif

    s->out = pscf->capability;
    return NJT_OK;
}


static njt_int_t
njt_mail_pop3_stls(njt_mail_session_t *s, njt_connection_t *c)
{
#if (NJT_MAIL_SSL)
    njt_mail_ssl_conf_t  *sslcf;

    if (c->ssl == NULL) {
        sslcf = njt_mail_get_module_srv_conf(s, njt_mail_ssl_module);
        if (sslcf->starttls) {
            s->buffer->pos = s->buffer->start;
            s->buffer->last = s->buffer->start;
            c->read->handler = njt_mail_starttls_handler;
            return NJT_OK;
        }
    }

#endif

    return NJT_MAIL_PARSE_INVALID_COMMAND;
}


static njt_int_t
njt_mail_pop3_apop(njt_mail_session_t *s, njt_connection_t *c)
{
    njt_str_t                 *arg;
    njt_mail_pop3_srv_conf_t  *pscf;

#if (NJT_MAIL_SSL)
    if (njt_mail_starttls_only(s, c)) {
        return NJT_MAIL_PARSE_INVALID_COMMAND;
    }
#endif

    if (s->args.nelts != 2) {
        return NJT_MAIL_PARSE_INVALID_COMMAND;
    }

    pscf = njt_mail_get_module_srv_conf(s, njt_mail_pop3_module);

    if (!(pscf->auth_methods & NJT_MAIL_AUTH_APOP_ENABLED)) {
        return NJT_MAIL_PARSE_INVALID_COMMAND;
    }

    arg = s->args.elts;

    s->login.len = arg[0].len;
    s->login.data = njt_pnalloc(c->pool, s->login.len);
    if (s->login.data == NULL) {
        return NJT_ERROR;
    }

    njt_memcpy(s->login.data, arg[0].data, s->login.len);

    s->passwd.len = arg[1].len;
    s->passwd.data = njt_pnalloc(c->pool, s->passwd.len);
    if (s->passwd.data == NULL) {
        return NJT_ERROR;
    }

    njt_memcpy(s->passwd.data, arg[1].data, s->passwd.len);

    njt_log_debug2(NJT_LOG_DEBUG_MAIL, c->log, 0,
                   "pop3 apop: \"%V\" \"%V\"", &s->login, &s->passwd);

    s->auth_method = NJT_MAIL_AUTH_APOP;

    return NJT_DONE;
}


static njt_int_t
njt_mail_pop3_auth(njt_mail_session_t *s, njt_connection_t *c)
{
    njt_int_t                  rc;
    njt_mail_pop3_srv_conf_t  *pscf;

#if (NJT_MAIL_SSL)
    if (njt_mail_starttls_only(s, c)) {
        return NJT_MAIL_PARSE_INVALID_COMMAND;
    }
#endif

    pscf = njt_mail_get_module_srv_conf(s, njt_mail_pop3_module);

    if (s->args.nelts == 0) {
        s->out = pscf->auth_capability;
        s->state = 0;

        return NJT_OK;
    }

    rc = njt_mail_auth_parse(s, c);

    switch (rc) {

    case NJT_MAIL_AUTH_LOGIN:

        njt_str_set(&s->out, pop3_username);
        s->mail_state = njt_pop3_auth_login_username;

        return NJT_OK;

    case NJT_MAIL_AUTH_LOGIN_USERNAME:

        njt_str_set(&s->out, pop3_password);
        s->mail_state = njt_pop3_auth_login_password;

        return njt_mail_auth_login_username(s, c, 1);

    case NJT_MAIL_AUTH_PLAIN:

        njt_str_set(&s->out, pop3_next);
        s->mail_state = njt_pop3_auth_plain;

        return NJT_OK;

    case NJT_MAIL_AUTH_CRAM_MD5:

        if (!(pscf->auth_methods & NJT_MAIL_AUTH_CRAM_MD5_ENABLED)) {
            return NJT_MAIL_PARSE_INVALID_COMMAND;
        }

        if (njt_mail_auth_cram_md5_salt(s, c, "+ ", 2) == NJT_OK) {
            s->mail_state = njt_pop3_auth_cram_md5;
            return NJT_OK;
        }

        return NJT_ERROR;

    case NJT_MAIL_AUTH_EXTERNAL:

        if (!(pscf->auth_methods & NJT_MAIL_AUTH_EXTERNAL_ENABLED)) {
            return NJT_MAIL_PARSE_INVALID_COMMAND;
        }

        njt_str_set(&s->out, pop3_username);
        s->mail_state = njt_pop3_auth_external;

        return NJT_OK;
    }

    return rc;
}