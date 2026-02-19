#ifndef _SMI_H_
#define _SMI_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "smi_errors.h"
#include "smi_types.h"

SMIInterpreter* smi_interpreter_new();
void smi_interpreter_destroy(SMIInterpreter* interp);

int smi_interpreter_eval(SMIInterpreter* interp, const char* code);

const char** smi_interpreter_memory_keys_get(SMIInterpreter* interp);
void smi_interpreter_memory_keys_free(const char** keys);
uint16_t smi_interpreter_memory_value_get(SMIInterpreter* interp, const char* key);

void smi_last_error_data_get(SMIErrorData* error);

SMIDebugger* smi_debugger_new();
void smi_debugger_destroy(SMIDebugger* debugger);
int smi_debugger_load(SMIDebugger* debugger, const char* code);
int smi_debugger_next(SMIDebugger* debugger);
int smi_debugger_next_index_get(SMIDebugger* debugger);
int smi_debugger_next_line_get(SMIDebugger* debugger);
bool smi_debugger_has_next(SMIDebugger* debugger);
SMIInterpreter* smi_debugger_as_interpreter(SMIDebugger* debugger);

SMIMSDecompiler* smi_msdecompiler_new();
void smi_msdecompiler_destroy(SMIMSDecompiler* decompiler);
const char* smi_msdecompiler_decompile(SMIMSDecompiler* decompiler, const char* code, int size);

SMIMSCompiler* smi_mscompiler_new();
void smi_mscompiler_destroy(SMIMSCompiler* compiler);
const char* smi_mscompiler_compile(SMIMSCompiler* compiler, const char* code, int size, int* sizeout);

#ifdef __cplusplus
}
#endif

#endif
