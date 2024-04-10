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


#include "njt_http_opentelemetry_log.h"

njt_flag_t logState = false;

/* This will log the error only when logState(traceAsInfo) flag is set by user*/
void njt_writeTrace(njt_log_t *log, const char* funcName, const char* format, ...)
{
    char note[8192];
    va_list args;

    va_start(args, format);
    vsnprintf(note, sizeof(note), format, args);
    va_end(args);

    if (logState && log)
    {
        njt_log_error(NJT_LOG_INFO, log, 0, "mod_opentelemetry: %s: %s", funcName, note);
    }
}

/* This will always log the error irrespective of the logState(traceAsError) flag */
void njt_writeError(njt_log_t *log, const char* funcName, const char* format, ...)
{
    char note[8192];
    va_list args;

    va_start(args, format);
    vsnprintf(note, sizeof(note), format, args);
    va_end(args);

    if (log && funcName)
    {
        njt_log_error(NJT_LOG_ERR, log, 0, "mod_opentelemetry: %s: %s", funcName, note);
    }
}


/* This will always log the error irrespective of the logState(traceAsError) flag */
void njt_writeInfo(njt_log_t *log, const char* funcName, const char* format, ...)
{
    char note[8192];
    va_list args;

    va_start(args, format);
    vsnprintf(note, sizeof(note), format, args);
    va_end(args);

    if (log && funcName)
    {
        njt_log_error(NJT_LOG_INFO, log, 0, "mod_opentelemetry: %s: %s", funcName, note);
    }
}


/* This will always log the error irrespective of the logState(traceAsError) flag */
void njt_writeDebug(njt_log_t *log, const char* funcName, const char* format, ...)
{
    char note[8192];
    va_list args;

    va_start(args, format);
    vsnprintf(note, sizeof(note), format, args);
    va_end(args);

    if (log && funcName)
    {
        njt_log_error(NJT_LOG_DEBUG, log, 0, "mod_opentelemetry: %s: %s", funcName, note);
    }
}