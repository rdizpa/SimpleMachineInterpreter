import SMI from "./smi.js";

onmessage = (e) => {
    if (e.data.type === "run") {
        const smi = new SMI();

        let code;
        
        try {
            code = smi.eval(e.data.code);
        } catch {
            code = -1;
        }

        if (code !== 0) {
            postMessage({ type: "error", error: {
                code,
                description: (code !== -1) ? smi.getLastError() : ""
            } });

            smi.destroy();
            return;
        }

        const keys = smi.getMemoryKeys();
        const memory = {};

        for (const key of keys) {
            memory[key] = smi.getMemoryValue(key);
        }

        smi.destroy();

        postMessage({ type: "result", memory });
    }
};
