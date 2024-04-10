#include "njt_config.h"

static const njt_uint_t argument_number[] = {
  NJT_CONF_NOARGS, NJT_CONF_TAKE1, NJT_CONF_TAKE2, NJT_CONF_TAKE3,
  NJT_CONF_TAKE4,  NJT_CONF_TAKE5, NJT_CONF_TAKE6, NJT_CONF_TAKE7,
};

njt_int_t OtelNjtConfHandler(njt_conf_t* cf, njt_int_t last) {
  void *conf, **confp;

  njt_str_t* name = (njt_str_t*)cf->args->elts;

  njt_uint_t found = 0;

  for (njt_uint_t i = 0; cf->cycle->modules[i]; i++) {

    njt_command_t* cmd = cf->cycle->modules[i]->commands;
    if (cmd == NULL) {
      continue;
    }

    for (/* void */; cmd->name.len; cmd++) {

      if (name->len != cmd->name.len) {
        continue;
      }

      if (njt_strcmp(name->data, cmd->name.data) != 0) {
        continue;
      }

      found = 1;

      if (
        cf->cycle->modules[i]->type != NJT_CONF_MODULE &&
        cf->cycle->modules[i]->type != cf->module_type) {
        continue;
      }

      /* is the directive's location right ? */

      if (!(cmd->type & cf->cmd_type)) {
        continue;
      }

      if (!(cmd->type & NJT_CONF_BLOCK) && last != NJT_OK) {
        njt_conf_log_error(
          NJT_LOG_EMERG, cf, 0, "directive \"%s\" is not terminated by \";\"", name->data);
        return NJT_ERROR;
      }

      if ((cmd->type & NJT_CONF_BLOCK) && last != NJT_CONF_BLOCK_START) {
        njt_conf_log_error(
          NJT_LOG_EMERG, cf, 0, "directive \"%s\" has no opening \"{\"", name->data);
        return NJT_ERROR;
      }

      /* is the directive's argument count right ? */

      if (!(cmd->type & NJT_CONF_ANY)) {

        if (cmd->type & NJT_CONF_FLAG) {

          if (cf->args->nelts != 2) {
            goto invalid;
          }

        } else if (cmd->type & NJT_CONF_1MORE) {

          if (cf->args->nelts < 2) {
            goto invalid;
          }

        } else if (cmd->type & NJT_CONF_2MORE) {

          if (cf->args->nelts < 3) {
            goto invalid;
          }

        } else if (cf->args->nelts > NJT_CONF_MAX_ARGS) {

          goto invalid;

        } else if (!(cmd->type & argument_number[cf->args->nelts - 1])) {
          goto invalid;
        }
      }

      /* set up the directive's configuration context */

      conf = NULL;

      if (cmd->type & NJT_DIRECT_CONF) {
        conf = ((void**)cf->ctx)[cf->cycle->modules[i]->index];

      } else if (cmd->type & NJT_MAIN_CONF) {
        conf = &(((void**)cf->ctx)[cf->cycle->modules[i]->index]);

      } else if (cf->ctx) {
        confp = (void**)*(void**)((char*)cf->ctx + cmd->conf);

        if (confp) {
          conf = confp[cf->cycle->modules[i]->ctx_index];
        }
      }

      char* rv = cmd->set(cf, cmd, conf);

      if (rv == NJT_CONF_OK) {
        return NJT_OK;
      }

      if (rv == NJT_CONF_ERROR) {
        return NJT_ERROR;
      }

      njt_conf_log_error(NJT_LOG_EMERG, cf, 0, "\"%s\" directive %s", name->data, rv);

      return NJT_ERROR;
    }
  }

  if (found) {
    njt_conf_log_error(NJT_LOG_EMERG, cf, 0, "\"%s\" directive is not allowed here", name->data);

    return NJT_ERROR;
  }

  njt_conf_log_error(NJT_LOG_EMERG, cf, 0, "unknown directive \"%s\"", name->data);

  return NJT_ERROR;

invalid:

  njt_conf_log_error(
    NJT_LOG_EMERG, cf, 0, "invalid number of arguments in \"%s\" directive", name->data);

  return NJT_ERROR;
}
