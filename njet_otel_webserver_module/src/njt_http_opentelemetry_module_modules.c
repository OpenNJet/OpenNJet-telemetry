#include <njt_config.h>
#include <njt_core.h>

extern njt_module_t njt_otel_webserver_module;

njt_module_t* njt_modules[] = {
  &njt_otel_webserver_module,
  NULL,
};

char* njt_module_names[] = {
  "njt_otel_webserver_module",
  NULL,
};

char* njt_module_order[] = {
  NULL,
};