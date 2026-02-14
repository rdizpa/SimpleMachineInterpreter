#include "smi.h"

#include <string.h>

#include "core/interpreter.h"
#include "core/ms/decompiler.h"

struct SMI {
    smi::interpreter::Interpreter interp;
};

struct SMIMSDecompiler {
    smi::ms::decompiler::MSDecompiler decompiler;
};

SMI* smi_new() {
    return new SMI();
}

void smi_destroy(SMI* interp) {
    delete interp;
}

int smi_eval(SMI* interp, const char* code) {
    switch (interp->interp.eval(code)) {
        case smi::interpreter::INTERPRETER_OK:
            return SMI_OK;
        case smi::interpreter::INTERPRETER_ERR_INVALID_TOKEN:
            return SMI_ERR_INVALID_TOKEN;
        case smi::interpreter::INTERPRETER_ERR_UNDEFINED_LABEL:
            return SMI_ERR_UNDEFINED_LABEL;
        case smi::interpreter::INTERPRETER_ERR_UNEXPECTED_TOKEN:
            return SMI_ERR_UNEXPECTED_TOKEN;
        default:
            return SMI_ERR_UNKNOWN;
    }
}

const char** smi_memory_keys_get(SMI* interp) {
    int size = interp->interp.getMemoryKeys().size();
    int i = 0;

    const char** keys = new const char*[size + 1];

    for (auto& key : interp->interp.getMemoryKeys()) {
        keys[i] = (char*)malloc(key.size() + 1);

        strcpy((char*)keys[i++], key.c_str());
    }

    keys[i] = NULL;

    return keys;
}

void smi_memory_keys_free(const char** keys) {
    int i = 0;

    while (keys[i] != NULL) {
        free((char**)keys[i++]);
    }

    delete[] keys;
}

uint16_t smi_memory_value_get(SMI* interp, const char* key) {
    return interp->interp.getMemoryValue(key);
}

void smi_last_error_data_get(SMIErrorData* error) {
    const smi::error::ErrorData& _error = smi::error::getLastError();

    error->index = _error.index;
    error->line = _error.line;
    error->column = _error.column;
    error->length = _error.length;
    error->message = _error.message.c_str();
}

SMIMSDecompiler* smi_msdecompiler_new() {
    return new SMIMSDecompiler();
}

void smi_msdecompiler_destroy(SMIMSDecompiler* decompiler) {
    delete decompiler;
}

const char* smi_msdecompiler_decompile(SMIMSDecompiler* decompiler, const char* code, int size) {
    std::string result;

    if (decompiler->decompiler.decompile(code, size, result) != smi::ms::decompiler::DECOMPILER_OK) return NULL;

    char* ptr = (char*)malloc(result.length() + 1);
    strcpy(ptr, result.c_str());

    return ptr;
}

#ifdef EMSCRIPTEN
#include <emscripten/bind.h>

EMSCRIPTEN_BINDINGS(module) {
    emscripten::constant("SMI_OK", (int)SMI_OK);
    emscripten::constant("SMI_ERR_INVALID_TOKEN", (int)SMI_ERR_INVALID_TOKEN);
    emscripten::constant("SMI_ERR_UNDEFINED_LABEL", (int)SMI_ERR_UNDEFINED_LABEL);
    emscripten::constant("SMI_ERR_UNEXPECTED_TOKEN", (int)SMI_ERR_UNEXPECTED_TOKEN);
}
#endif
