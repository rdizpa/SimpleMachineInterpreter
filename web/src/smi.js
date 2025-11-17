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
        getLastError: () => Module.ccall("smi_last_error_get", "string", [], [])
    };
};

export default SMI;
export const SMIError = Module.SMIError;
