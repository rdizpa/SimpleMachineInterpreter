#ifndef _SMI_H_
#define _SMI_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#include "smi_errors.h"

typedef struct SMI SMI;

SMI* smi_new();
void smi_destroy(SMI* interp);

int smi_eval(SMI* interp, const char* code);

const char** smi_memory_keys_get(SMI* interp);
void smi_memory_keys_free(const char** keys);
uint16_t smi_memory_value_get(SMI* interp, const char* key);

void smi_last_error_data_get(SMIErrorData* error);

#ifdef __cplusplus
}
#endif

#endif
