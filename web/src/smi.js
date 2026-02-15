import createModule from "./smiwasmout.js";
import smiwasmout from "/smiwasmout.wasm?url";

const Module = await createModule({
    locateFile: (path) => smiwasmout
});

function SMI() {
    const _SMI = Module._smi_new();

    return {
        destroy: () => Module._smi_destroy(_SMI),
        eval: (code) => Module.ccall("smi_eval", "number", ["number", "string"], [_SMI, code]),
        getMemoryValue: (key) => Module.ccall("smi_memory_value_get", "number", ["number", "string"], [_SMI, key]),
        getMemoryKeys: () => {
            const keys = [];

            let arrayPointer = Module.ccall("smi_memory_keys_get", "number", ["number"], [_SMI]);

            while (Module.getValue(arrayPointer, "i32") != 0) {
                keys.push(Module.UTF8ToString(Module.getValue(arrayPointer, "i32")));
                arrayPointer += 4;
            }

            Module.ccall("smi_memory_keys_free", null, ["number"], [arrayPointer]);

            return keys;
        }
    };
};

SMI.getLastErrorData = () => {
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

            const result = new Uint8Array(Module.HEAP8.buffer, ptr, size);

            Module._free(ptr);

            return result;
        }
    };
}

const SMIError = {};

Object.keys(Module).forEach((key) => {
    if (key.startsWith("SMI_"))
        SMIError[key] = Module[key];
});

export default SMI;
export { SMICompiler, SMIDecompiler, SMIError };
