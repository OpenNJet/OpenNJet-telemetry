#include "script.h"

bool CompileScript(njt_conf_t* conf, njt_str_t pattern, NjtCompiledScript* script) {
  script->pattern = pattern;
  script->lengths = nullptr;
  script->values = nullptr;

  njt_uint_t numVariables = njt_http_script_variables_count(&script->pattern);

  if (numVariables == 0) {
    return true;
  }

  njt_http_script_compile_t compilation;
  njt_memzero(&compilation, sizeof(compilation));
  compilation.cf = conf;
  compilation.source = &script->pattern;
  compilation.lengths = &script->lengths;
  compilation.values = &script->values;
  compilation.variables = numVariables;
  compilation.complete_lengths = 1;
  compilation.complete_values = 1;

  return njt_http_script_compile(&compilation) == NJT_OK;
}

bool CompileScriptAttribute(
  njt_conf_t* conf, ScriptAttributeDeclaration declaration,
  CompiledScriptAttribute* compiledAttribute) {
  compiledAttribute->type = declaration.type;

  if (!CompileScript(conf, declaration.attribute, &compiledAttribute->key)) {
    return false;
  }

  return CompileScript(conf, declaration.script, &compiledAttribute->value);
}
