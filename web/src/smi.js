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
        },
        getLastErrorData: () => {
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
        }
    };
};

const SMIError = {};

Object.keys(Module).forEach((key) => {
    if (key.startsWith("SMI_"))
        SMIError[key] = Module[key];
});

export default SMI;
export { SMIError };
