#ifndef _SMI_H_
#define _SMI_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "smi_errors.h"

typedef struct SMI SMI;
typedef struct SMIMSDecompiler SMIMSDecompiler;

SMI* smi_new();
void smi_destroy(SMI* interp);

int smi_eval(SMI* interp, const char* code);

const char** smi_memory_keys_get(SMI* interp);
void smi_memory_keys_free(const char** keys);
uint16_t smi_memory_value_get(SMI* interp, const char* key);

void smi_last_error_data_get(SMIErrorData* error);

SMIMSDecompiler* smi_msdecompiler_new();
void smi_msdecompiler_destroy(SMIMSDecompiler* decompiler);
const char* smi_msdecompiler_decompile(SMIMSDecompiler* decompiler, const char* code, int size);

#ifdef __cplusplus
}
#endif

#endif
