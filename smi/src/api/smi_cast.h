#ifndef _SMI_CAST_H_
#define _SMI_CAST_H_

#include "core/debugger.h"
#include "core/interpreter.h"
#include "core/ms/compiler.h"
#include "core/ms/decompiler.h"
#include "smi_types.h"

smi::interpreter::Interpreter* cast(SMIInterpreter* interp);
SMIInterpreter* cast(smi::interpreter::Interpreter* interp);

smi::debugger::Debugger* cast(SMIDebugger* debugger);
SMIDebugger* cast(smi::debugger::Debugger* debugger);

smi::ms::compiler::MSCompiler* cast(SMIMSCompiler* compiler);
SMIMSCompiler* cast(smi::ms::compiler::MSCompiler* compiler);

smi::ms::decompiler::MSDecompiler* cast(SMIMSDecompiler* decompiler);
SMIMSDecompiler* cast(smi::ms::decompiler::MSDecompiler* decompiler);

#endif
