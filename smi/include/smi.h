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
int smi_interpreter_executed_instructions_get(SMIInterpreter* interp);

void smi_last_error_data_get(SMIErrorData* error);

SMIDebugger* smi_debugger_new();
void smi_debugger_destroy(SMIDebugger* debugger);
int smi_debugger_load(SMIDebugger* debugger, const char* code);
int smi_debugger_next(SMIDebugger* debugger);
int smi_debugger_next_index_get(SMIDebugger* debugger);
int smi_debugger_next_line_get(SMIDebugger* debugger);
bool smi_debugger_has_next(SMIDebugger* debugger);
bool smi_debugger_cmp_get(SMIDebugger* debugger);
SMIInterpreter* smi_debugger_as_interpreter(SMIDebugger* debugger);

SMIMSDecompiler* smi_msdecompiler_new();
void smi_msdecompiler_destroy(SMIMSDecompiler* decompiler);
const char* smi_msdecompiler_decompile(SMIMSDecompiler* decompiler, const char* code, int size);

SMIMSCompiler* smi_mscompiler_new();
void smi_mscompiler_destroy(SMIMSCompiler* compiler);
const char* smi_mscompiler_compile(SMIMSCompiler* compiler, const char* code, int size, int* sizeout);
const unsigned int* smi_mscopiler_linemap_get(SMIMSCompiler* compiler);

SMIMSVM* smi_msvm_new();
void smi_msvm_destroy(SMIMSVM* vm);
void smi_msvm_loadms(SMIMSVM* vm, unsigned char* ms);
int smi_msvm_execute_next(SMIMSVM* vm);
unsigned int smi_msvm_pc_get(SMIMSVM* vm);
uint16_t smi_msvm_ir_get(SMIMSVM* vm);
uint8_t smi_msvm_zf_get(SMIMSVM* vm);
void smi_msvm_pc_set(SMIMSVM* vm, unsigned int pc);
void smi_msvm_zf_set(SMIMSVM* vm, uint8_t zf);
const uint16_t* smi_msvm_memory_get(SMIMSVM* vm);
uint16_t smi_msvm_memory_value_get(SMIMSVM* vm, uint8_t pos);
const char** smi_msvm_labels_get(SMIMSVM* vm);
void smi_msvm_labels_free(const char** labels);
uint16_t smi_msvm_label_get(SMIMSVM* vm, const char* label);

#ifdef __cplusplus
}
#endif

#endif
