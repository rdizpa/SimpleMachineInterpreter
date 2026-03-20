import SMI, { SMICompiler, SMIVM } from "./smi";
import "./ms.css";

const opcodes = [
    "ADD",
    "CMP",
    "MOV",
    "BEQ"
];

function debuggerMsShowMemory(vm, labels, memory) {
    const output = document.querySelector(".output .content");
    output.innerHTML = "";

    for (const key in labels) {
        const value = memory[key];
        output.insertAdjacentHTML("beforeend",
            `<div class="row"><div>${labels[key]}</div><div>0x${value.toString(16)}</div><div>${value}</div></div>`
        );
    }

    output.insertAdjacentHTML("beforeend", `<div class="row-header"><h4>Register</h4><h4>Value</h4></div>`);
    output.insertAdjacentHTML("beforeend", `<div class="row"><div>ZF</div><div>${vm.getZF()}</div></div>`);
    output.insertAdjacentHTML("beforeend", `<div class="row"><div>PC</div><div>${vm.getPC()}</div></div>`);
    output.insertAdjacentHTML("beforeend", `<div class="row"><div>IR</div><div>${vm.getIR().toString(16).padStart(4, "0").toUpperCase()}</div></div>`);

    //output.insertAdjacentHTML("beforeend", `<div class="row-executed-instructions">Total executed instructions: ${smiDebugger.getExecutedInstructions()}</div>`);
}

class SmiMsComponent extends HTMLElement {
    constructor() {
        super();
        //this.attachShadow({ mode: "open" });
        this.memory = [];
        this.labels = {};
        this.vm = null;
        this.lineMap = [];
        this.breakpoints = new Set();
    }

    firstRender() {
        this.innerHTML = `<div class="smi-ms-container"></div>`;

        this.updateBreakpoints();

        const output = this.querySelector(".smi-ms-container");
        output.insertAdjacentHTML("beforeend", `<div class="smi-ms-line smi-ms-line-header">
            <span class="col-1" title="Position">Pos</span>
            <span class="col-2">Value</span>
            <span class="col-3">Repr</span>
        </div>`);

        for (let i = 0; i < 128; i++) {
            const value = this.memory[i] || 0;

            output.insertAdjacentHTML("beforeend", `<div class="smi-ms-line${
                this.breakpoints.has(i) ? " smi-ms-line-breakpoint" : ""
            }">
                <span class="col-1">${i.toString(16).padStart(2, "0").toUpperCase()}</span>
                <span class="col-2">${value.toString(16).padStart(4, "0").toUpperCase()}</span>
                <span class="col-3 label">${(this.labels[i] ? this.labels[i] + ": " : "").padStart(7, " ")}</span>
                <span class="col-4">${this.reprValue(value)}</span>
            </div>`);
        }
    }

    render() {
        if (!this.vm)
            return;

        this.updateBreakpoints();

        const output = this.querySelector(".smi-ms-container");

        for (let i = 0; i < 128; i++) {
            const value = this.memory[i] || 0;
            const child = output.children[i + 1];

            if (this.breakpoints.has(i)) {
                child.classList.add("smi-ms-line-breakpoint");
            } else {
                child.classList.remove("smi-ms-line-breakpoint");
            }

            if (child.children[1].textContent === value.toString(16).padStart(4, "0").toUpperCase())
                continue;

            child.children[0].textContent = i.toString(16).padStart(2, "0").toUpperCase();
            child.children[1].textContent = value.toString(16).padStart(4, "0").toUpperCase();
            child.children[2].textContent = (this.labels[i] ? this.labels[i] + ": " : "").padStart(7, " ");
            child.children[3].innerHTML = this.reprValue(value);
        }
    }

    updateBreakpoints() {
        this.breakpoints.clear();
        
        for (const breakpoint of window.breakpoints) {
            const pos = this.lineMap.indexOf(breakpoint);

            if (pos !== -1) {
                this.breakpoints.add(pos);
            }
        }
    }

    reprValue(value) {
        const opcode = (value >> 14) & 0x03;
        let op1 = (value >> 7) & 0x7F;
        let op2 = value & 0x7F;
        const instr = opcodes[opcode];

        op1 = this.labels[op1]
            ? `<span class="var">${this.labels[op1]}</span>`
            : `<span>${op1.toString(16).padStart(2, "0").toUpperCase()}</span>`;
        op2 = this.labels[op2]
            ? `<span class="var">${this.labels[op2]}</span>`
            : `<span>${op2.toString(16).padStart(2, "0").toUpperCase()}</span>`;

        if (instr === "BEQ")
            return `<span class="instruction">${instr}</span> ${op2}`;

        return `<span class="instruction">${instr}</span> ${op1}, ${op2}`;
    }
    
    connectedCallback() { this.firstRender(); }
    disconnectedCallback() { if (this.vm) this.vm.destroy(); }
    attributeChangedCallback() { this.render(); }

    loadCode(code) {
        const compiler = SMICompiler();
        const ms = compiler.compile(code);
        this.lineMap = compiler.getLineMap();
        compiler.destroy();

        if (!ms) {
            window.showError(SMI.getLastErrorData());
            return;
        }

        if (this.vm) this.vm.destroy();

        this.vm = SMIVM();
        this.vm.loadMS(ms);
        const labels = this.vm.getLabels();
        this.labels = {};

        for (const label of labels) {
            this.labels[this.vm.getLabel(label)] = label;
        }

        this.memory = this.vm.getMemory();

        debuggerMsShowMemory(this.vm, this.labels, this.memory);

        this.style.setProperty("--debugger-line-pos", `${this.vm.getPC()}`);

        this.render();
    }

    step() {
        if (!this.vm) return;

        this.vm.executeNext();

        this.render();

        debuggerMsShowMemory(this.vm, this.labels, this.memory);

        this.style.setProperty("--debugger-line-pos", `${this.vm.getPC()}`);
    }

    async runUntilBreakpoint(timeout = 0) {
        let lastTime = 0;

        while (this.vm) {
            if (this.vm.executeNext() !== 0) {
                break;
            }

            if (timeout > 0 || (performance.now() - lastTime) > 10) {
                this.render();
                debuggerMsShowMemory(this.vm, this.labels, this.memory);
                this.style.setProperty("--debugger-line-pos", `${this.vm.getPC()}`);
            }

            if (this.breakpoints.has(this.vm.getPC()))
                break;

            if (timeout > 0 || (performance.now() - lastTime) > 10) {
                lastTime = performance.now();
                await new Promise((res) => setTimeout(res, timeout));
            }
        }

        if (this.vm) {
            debuggerMsShowMemory(this.vm, this.labels, this.memory);
            this.style.setProperty("--debugger-line-pos", `${this.vm.getPC()}`);
            this.render();
        }
    }

    stop() {
        this.vm.destroy();
        this.vm = null;
    }
};

customElements.define("smi-ms", SmiMsComponent);

document.getElementById("ms-mode-checkbox").addEventListener("input", (ev) => {
    window.isMsModeActive = ev.target.checked;

    if (window.isMsModeActive) {
        if (!document.getElementById("smi-ms")) {
            const smiMs = document.createElement("smi-ms");
            smiMs.id = "smi-ms";
            smiMs.style.width = "400px";
            smiMs.style.setProperty("--debugger-line-pos", "0");
            document.querySelector(".editor-layout .editor-separator").insertAdjacentElement("afterend", smiMs);
        }
    } else {
        if (document.getElementById("smi-ms")) {
            document.getElementById("smi-ms").remove();
        }
    }
});

document.getElementById("debug").addEventListener("click", () => {
    if (!window.isMsModeActive) return;

    document.getElementById("smi-ms").loadCode(editor.value);
});

document.getElementById("debug-step").addEventListener("click", () => {
    if (!window.isMsModeActive) return;

    document.getElementById("smi-ms").step();
});

document.getElementById("debug-stop").addEventListener("click", () => {
    if (!window.isMsModeActive) return;

    document.getElementById("smi-ms").stop();
});

document.getElementById("run").addEventListener("click", () => {
    if (!window.isMsModeActive) return;

    if (document.getElementById("toolbar").classList.contains("toolbar-debug")) {
        document.getElementById("smi-ms").runUntilBreakpoint();
        return;
    }

    document.getElementById("debug").click();
});
