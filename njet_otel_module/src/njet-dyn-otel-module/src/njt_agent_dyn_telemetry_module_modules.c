#include <njt_config.h>
#include <njt_core.h>

extern njt_module_t njt_agent_dyn_telemetry_module;

njt_module_t* njt_modules[] = {
  &njt_agent_dyn_telemetry_module,
  NULL,
};

char* njt_module_names[] = {
  "njt_agent_dyn_telemetry_module",
  NULL,
};

char* njt_module_order[] = {
  NULL,
};
