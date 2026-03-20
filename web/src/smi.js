import createModule from "./smiwasmout.js";
import smiwasmout from "/smiwasmout.wasm?url";

const Module = await createModule({
    locateFile: (path) => smiwasmout
});

function SMIInterpreter() {
    const _SMIInterpreter = Module._smi_interpreter_new();

    return {
        destroy: () => Module._smi_interpreter_destroy(_SMIInterpreter),
        eval: (code) => Module.ccall("smi_interpreter_eval", "number", ["number", "string"], [_SMIInterpreter, code]),
        getMemoryValue: (key) => Module.ccall("smi_interpreter_memory_value_get", "number", ["number", "string"], [_SMIInterpreter, key]),
        getMemoryKeys: () => {
            const keys = [];

            let arrayPointer = Module.ccall("smi_interpreter_memory_keys_get", "number", ["number"], [_SMIInterpreter]);

            while (Module.getValue(arrayPointer, "i32") != 0) {
                keys.push(Module.UTF8ToString(Module.getValue(arrayPointer, "i32")));
                arrayPointer += 4;
            }

            Module.ccall("smi_interpreter_memory_keys_free", null, ["number"], [arrayPointer]);

            return keys;
        },
        getExecutedInstructions: () => Module.ccall("smi_interpreter_executed_instructions_get", "number", ["number"], [_SMIInterpreter])
    };
};

const getLastErrorData = () => {
    const _errorData = Module._malloc(20);
    Module.ccall("smi_last_error_data_get", null, ["number"], [_errorData]);

    const errorData = {
        index: Module.getValue(_errorData, "i32"),
        line: Module.getValue(_errorData + 4, "i32"),
        column: Module.getValue(_errorData + 8, "i32"),
        length: Module.getValue(_errorData + 12, "i32"),
        message: Module.UTF8ToString(Module.getValue(_errorData + 16, "i32"))
    };

    Module._free(_errorData);

    return errorData;
};

function SMIDebugger() {
    const _SMIDebugger = Module._smi_debugger_new();
    const _SMIDebuggerInterp = Module.ccall("smi_debugger_as_interpreter", "number", ["number"], [_SMIDebugger]);

    return {
        destroy: () => Module._smi_debugger_destroy(_SMIDebugger),
        load: (code) => Module.ccall("smi_debugger_load", "number", ["number", "string"], [_SMIDebugger, code]),
        next: () => Module.ccall("smi_debugger_next", "number", ["number"], [_SMIDebugger]),
        hasNext: () => Module.ccall("smi_debugger_has_next", "boolean", ["number"], [_SMIDebugger]),
        getNextIndex: () => Module.ccall("smi_debugger_next_index_get", "number", ["number"], [_SMIDebugger]),
        getNextLine: () => Module.ccall("smi_debugger_next_line_get", "number", ["number"], [_SMIDebugger]),
        getMemoryValue: (key) => Module.ccall("smi_interpreter_memory_value_get", "number", ["number", "string"], [_SMIDebuggerInterp, key]),
        getMemoryKeys: () => {
            const keys = [];

            let arrayPointer = Module.ccall("smi_interpreter_memory_keys_get", "number", ["number"], [_SMIDebuggerInterp]);

            while (Module.getValue(arrayPointer, "i32") != 0) {
                keys.push(Module.UTF8ToString(Module.getValue(arrayPointer, "i32")));
                arrayPointer += 4;
            }

            Module.ccall("smi_interpreter_memory_keys_free", null, ["number"], [arrayPointer]);

            return keys;
        },
        getCmp: () => Module.ccall("smi_debugger_cmp_get", "boolean", ["number"], [_SMIDebugger]),
        getExecutedInstructions: () => Module.ccall("smi_interpreter_executed_instructions_get", "number", ["number"], [_SMIDebuggerInterp])
    };
}

function SMIDecompiler() {
    const _SMIDecompiler = Module._smi_msdecompiler_new();

    return {
        destroy: () => Module._smi_msdecompiler_destroy(_SMIDecompiler),
        decompile: (data) => {
            const result = Module.ccall(
                "smi_msdecompiler_decompile",
                "number",
                ["number", "array", "number"],
                [_SMIDecompiler, data, data.byteLength]
            );

            if (result === 0) {
                return "";
            }

            const code = Module.UTF8ToString(result);
            Module._free(result);

            return code;
        }
    };
}

function SMICompiler() {
    const _SMICompiler = Module._smi_mscompiler_new();

    return {
        destroy: () => Module._smi_mscompiler_destroy(_SMICompiler),
        compile: (code) => {
            const outSize = Module._malloc(4);

            const ptr = Module.ccall(
                "smi_mscompiler_compile",
                "number",
                ["number", "string", "number", "number"],
                [_SMICompiler, code, code.length, outSize]
            );

            const size = Module.getValue(outSize, "i32");
            Module._free(outSize);

            if (ptr === 0)
                return null;

            const result = new Uint8Array(Module.HEAP8.buffer.slice(ptr, ptr + size));

            Module._free(ptr);

            return result;
        },
        getLineMap: () => {
            const lineMap = Array(128).fill(-1);

            const ptr = Module.ccall("smi_mscopiler_linemap_get", "number", ["number"], [_SMICompiler]);

            for (let pos = 0; pos < 128; pos++) {
                lineMap[pos] = Module.getValue(ptr + pos * 4, "i32");
            }

            return lineMap;
        }
    };
}

function SMIVM() {
    const _SMIVM = Module._smi_msvm_new();

    return {
        destroy: () => Module._smi_msvm_destroy(_SMIVM),
        loadMS: (ms) => Module.ccall("smi_msvm_loadms", null, ["number", "array"], [_SMIVM, ms]),
        executeNext: () => Module.ccall("smi_msvm_execute_next", "number", ["number"], [_SMIVM]),
        getPC: () => Module.ccall("smi_msvm_pc_get", "number", ["number"], [_SMIVM]),
        getIR: () => Module.ccall("smi_msvm_ir_get", "number", ["number"], [_SMIVM]),
        getZF: () => Module.ccall("smi_msvm_zf_get", "number", ["number"], [_SMIVM]),
        setPC: (pc) => Module.ccall("smi_msvm_pc_set", null, ["number", "number"], [_SMIVM, pc]),
        setZF: (zf) => Module.ccall("smi_msvm_zf_set", null, ["number", "number"], [_SMIVM, zf]),
        getLabels: () => {
            const keys = [];

            let arrayPointer = Module.ccall("smi_msvm_labels_get", "number", ["number"], [_SMIVM]);

            while (Module.getValue(arrayPointer, "i32") != 0) {
                keys.push(Module.UTF8ToString(Module.getValue(arrayPointer, "i32")));
                arrayPointer += 4;
            }

            Module.ccall("smi_msvm_labels_free", null, ["number"], [arrayPointer]);

            return keys;
        },
        getLabel: (label) => Module.ccall("smi_msvm_label_get", "number", ["number", "string"], [_SMIVM, label]),
        getMemory: () => {
            const ptr = Module.ccall("smi_msvm_memory_get", "number", ["number"], [_SMIVM]);
            return Module.HEAPU16.subarray(ptr / 2, ptr / 2 + 128);
        },
        getMemoryValue: (pos) => Module.ccall("smi_msvm_memory_value_get", "number", ["number", "number"], [_SMIVM, pos])
    };
}

const SMIError = {};

Object.keys(Module).forEach((key) => {
    if (key.startsWith("SMI_"))
        SMIError[key] = Module[key];
});

export default { getLastErrorData };
export { SMIInterpreter, SMIDebugger, SMICompiler, SMIDecompiler, SMIVM, SMIError };
