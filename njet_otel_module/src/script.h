#pragma once

extern "C" {
#include <njt_config.h>
#include <njt_core.h>
#include <njt_http.h>
}

#include "njt_utils.h"

struct NjtCompiledScript {
  njt_str_t pattern = njt_null_string;
  njt_array_t* lengths = nullptr;
  njt_array_t* values = nullptr;

  bool Run(njt_http_request_t* req, njt_str_t* result) {
    if (!lengths) {
      *result = pattern;
      return true;
    }

    if (!njt_http_script_run(req, result, lengths->elts, 0, values->elts)) {
      *result = njt_null_string;
      return false;
    }

    return true;
  }

  bool IsEmpty() {
    return pattern.len == 0;
  }
};

enum ScriptAttributeType {
  ScriptAttributeInt,
  ScriptAttributeString,
};

struct ScriptAttributeDeclaration {
  ScriptAttributeDeclaration(
    opentelemetry::nostd::string_view attribute, opentelemetry::nostd::string_view script, ScriptAttributeType type = ScriptAttributeString)
    : attribute(ToNjtString(attribute)), script(ToNjtString(script)), type(type) {}

  ScriptAttributeDeclaration(njt_str_t attribute, njt_str_t script)
    : attribute(attribute), script(script), type(ScriptAttributeString) {}

  njt_str_t attribute;
  njt_str_t script;
  ScriptAttributeType type;
};

struct CompiledScriptAttribute {
  NjtCompiledScript key;
  NjtCompiledScript value;
  ScriptAttributeType type;
};

bool CompileScript(njt_conf_t* conf, njt_str_t pattern, NjtCompiledScript* script);
bool CompileScriptAttribute(
  njt_conf_t* conf, ScriptAttributeDeclaration declaration,
  CompiledScriptAttribute* compiledAttribute);
