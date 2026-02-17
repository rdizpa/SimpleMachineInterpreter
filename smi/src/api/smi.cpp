#include "smi.h"

#include <string.h>

#include "smi_cast.h"

SMIInterpreter* smi_interpreter_new() {
    return cast(new smi::interpreter::Interpreter());
}

void smi_interpreter_destroy(SMIInterpreter* interp) {
    delete cast(interp);
}

int smi_interpreter_eval(SMIInterpreter* interp, const char* code) {
    switch (cast(interp)->eval(code)) {
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

const char** smi_interpreter_memory_keys_get(SMIInterpreter* interp) {
    int size = cast(interp)->getMemoryKeys().size();
    int i = 0;

    const char** keys = new const char*[size + 1];

    for (auto& key : cast(interp)->getMemoryKeys()) {
        keys[i] = (char*)malloc(key.size() + 1);

        strcpy((char*)keys[i++], key.c_str());
    }

    keys[i] = NULL;

    return keys;
}

void smi_interpreter_memory_keys_free(const char** keys) {
    int i = 0;

    while (keys[i] != NULL) {
        free((char**)keys[i++]);
    }

    delete[] keys;
}

uint16_t smi_interpreter_memory_value_get(SMIInterpreter* interp, const char* key) {
    return cast(interp)->getMemoryValue(key);
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
    return cast(new smi::ms::decompiler::MSDecompiler());
}

void smi_msdecompiler_destroy(SMIMSDecompiler* decompiler) {
    delete cast(decompiler);
}

const char* smi_msdecompiler_decompile(SMIMSDecompiler* decompiler, const char* code, int size) {
    std::string result;

    if (cast(decompiler)->decompile(code, size, result) != smi::ms::decompiler::DECOMPILER_OK) return NULL;

    char* ptr = (char*)malloc(result.length() + 1);
    strcpy(ptr, result.c_str());

    return ptr;
}

SMIMSCompiler* smi_mscompiler_new() {
    return cast(new smi::ms::compiler::MSCompiler());
}

void smi_mscompiler_destroy(SMIMSCompiler* compiler) {
    delete cast(compiler);
}

const char* smi_mscompiler_compile(SMIMSCompiler* compiler, const char* code, int size, int* sizeout) {
    std::string result;

    if (cast(compiler)->compile(std::string(code, size), result) != smi::ms::compiler::COMPILER_OK) return NULL;

    char* ptr = (char*)malloc(result.length());
    memcpy(ptr, result.c_str(), result.length());

    *sizeout = result.length();

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
