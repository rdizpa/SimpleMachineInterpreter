#include "smi_cast.h"

smi::interpreter::Interpreter* cast(SMIInterpreter* interp) {
    return reinterpret_cast<smi::interpreter::Interpreter*>(interp);
}

SMIInterpreter* cast(smi::interpreter::Interpreter* interp) {
    return reinterpret_cast<SMIInterpreter*>(interp);
}

smi::ms::compiler::MSCompiler* cast(SMIMSCompiler* compiler) {
    return reinterpret_cast<smi::ms::compiler::MSCompiler*>(compiler);
}

SMIMSCompiler* cast(smi::ms::compiler::MSCompiler* compiler) {
    return reinterpret_cast<SMIMSCompiler*>(compiler);
}

smi::ms::decompiler::MSDecompiler* cast(SMIMSDecompiler* decompiler) {
    return reinterpret_cast<smi::ms::decompiler::MSDecompiler*>(decompiler);
}

SMIMSDecompiler* cast(smi::ms::decompiler::MSDecompiler* decompiler) {
    return reinterpret_cast<SMIMSDecompiler*>(decompiler);
}
