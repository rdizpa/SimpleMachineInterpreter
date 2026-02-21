import "./style.css"

import SMI, { SMIDebugger, SMICompiler, SMIDecompiler } from "./smi";
import SMIWorker from "./smi-worker.js?worker";

const editor = document.getElementById("editor");
const editorHighlight = document.getElementById("editor-highlight");
const output = document.querySelector(".output .content");
const runBtn = document.getElementById("run");
const stopBtn = document.getElementById("stop");
const error = document.getElementById("error");
const lineNumberColumn = document.querySelector(".line-number-column");
const saveFileDialog = document.getElementById("save-file-dialog");
const saveFileForm = document.querySelector("form.sfd-container");

setTimeout(() => {
    const params = new URLSearchParams(document.location.href.split("?")[1]);

    if (params.has("d")) {
        setEditorContent(window.atob(params.get("d")));
    }
}, 0);

const INSTRUCTIONS = ["MOV", "ADD", "CMP", "BEQ"];
const highlights = {};

highlights["ERROR"] = new Highlight();
CSS.highlights.set("error", highlights["ERROR"]);

const normalize = () => {
    const selStart = editor.selectionStart;

    const matches = editor.value.matchAll(new RegExp(`\\b(${INSTRUCTIONS.join("|")})\\b`, "gdi"));

    for (const m of matches) {
        if (m[1] !== m[1].toUpperCase()) {
            editor.setRangeText(m[1].toUpperCase(), m.indices[1][0], m.indices[1][1], "end");
        }
    }

    for (const m of editor.value.matchAll(/\b([A-Z0-9a-z]+)\b:/gd)) {
        for (const ml of editor.value.matchAll(new RegExp(`\\b(${m[1]})\\b([^:]|$)`, "gdi"))) {
            if (ml[1] !== m[1]) {
                editor.setRangeText(m[1], ml.indices[1][0], ml.indices[1][1], "end");
            }
        }
    }

    editor.selectionStart = selStart;
    editor.selectionEnd = selStart;
};

const getNodeAtPosition = (lineNode, pos, endNode = false) => {
    let i = 0;
    let lastI = 0;
    let nodeIndex = 0;

    while (i <= pos && nodeIndex < lineNode.childNodes.length) {
        lastI = i;
        i += lineNode.childNodes[nodeIndex].textContent.length;

        if (i > pos)
            break;

        if (endNode && i == pos)
            break;

        nodeIndex++;
    }

    if (nodeIndex >= lineNode.childNodes.length)
        return null;
    
    return {
        node: lineNode.childNodes[nodeIndex],
        i: lastI
    };
};

const INSTRUCTIONS_RE = new RegExp(`\\b(${INSTRUCTIONS.join("|")})\\b`, "g");
const NUMBER_RE = /\b:[ \t]*((?:0[xX])?[0-9A-Fa-f]+)\b/gd;
const LABEL_RE = /\b([A-Z0-9a-z]+)\b:/gd;
const COMMENT_RE = /(;[^\n]*)/gd;
const NEWLINE_RE = /(\n)/gd;
const COLORIZE_RE = new RegExp(`(?:\\b(${INSTRUCTIONS.join("|")})\\b|\\b([A-Z0-9a-z]+)\\b:|(;[^\\n]*)|(\\n))`, "gd");

const colorize = () => {
    const code = editor.value;
    const res = [];
    const labels = new Set();

    for (const m of code.matchAll(COLORIZE_RE)) {
        const [_, instruction, label, comment, newline] = m;

        const type = instruction ? "instruction" : label ? "label" : comment ? "comment" : "newline";
        const i = instruction ? 1 : label ? 2 : comment ? 3 : 4;

        res.push({
            type,
            offset: m.indices[i][0],
            length : m[i].length,
            start: m.indices[i][0],
            end: m.indices[i][1]
        });

        if (type === "label") {
            labels.add(m[i]);
        }
    }

    for (const m of code.matchAll(NUMBER_RE)) {
        res.push({
            type: "number",
            offset: m.indices[1][0],
            length : m[1].length,
            start: m.indices[1][0],
            end: m.indices[1][1]
        });
    }

    for (const label of labels) {
        for (const l of code.matchAll(new RegExp(`\\b(${label})\\b([^:]|$)`, "gd"))) {
            res.push({
                type: "var",
                offset: l.indices[1][0],
                length : l[1].length,
                start: l.indices[1][0],
                end: l.indices[1][1]
            });
        }
    }

    res.sort((a, b) => a.start - b.start);

    let codeResult = `<div class="editor-line">`;
    let lastIndex = 0;

    for (const tk of res) {
        if (tk.start < lastIndex)
            continue;

        codeResult += `<span>${code.substring(lastIndex, tk.start)}</span>`;

        if (tk.type === "newline") {
            codeResult += `</div><div class="editor-line">`;
        } else {
            codeResult += `<span class="${tk.type}">${code.substring(tk.start, tk.end)}</span>`;
        }

        lastIndex = tk.end;
    }

    codeResult += `<span>${code.substring(lastIndex)}</span></div>`;

    editorHighlight.innerHTML = codeResult;
}

const updateLineNumberColumn = () => {
    const lines = editor.value.split("\n").length;
    const currentLines = parseInt(lineNumberColumn.dataset.lines) || 1;
    
    if (lines == currentLines)
        return;

    let str = "";

    for (let i = 0; i < lines; i++)
        str += `<span data-line="${i + 1}">${i + 1}</span>`;

    lineNumberColumn.innerHTML = str;
    lineNumberColumn.dataset.lines = lines;

    lineNumberColumn.onclick = (ev) => {
        if (ev.target.tagName !== "SPAN")
            return;

        toggleBreakpoint(parseInt(ev.target.dataset.line));
        ev.target.dataset.breakpoint = (hasBreakpoint(parseInt(ev.target.dataset.line)) ? "true" : "false");
    };
};

const setEditorContent = (content, updateURL = false) => {
    editor.value = content;
    updateLineNumberColumn();
    colorize();

    if (updateURL)
        window.history.replaceState({}, "", "?d=" + window.btoa(editor.value));
};

editor.addEventListener("scroll", (ev) => {
    lineNumberColumn.scrollTop = editor.scrollTop;
    editorHighlight.scrollTop = editor.scrollTop;
    editorHighlight.scrollLeft = editor.scrollLeft;
});

editor.addEventListener("input", (ev) => {
    clearError();
    normalize();
    updateLineNumberColumn();
    colorize();

    window.history.replaceState({}, "", "?d=" + window.btoa(editor.value));
});

let ranges = [];

function clearError() {
    if (error.style.display === "none")
        return;
    
    error.style.display = "none";
    error.style.transform = "";
    error.classList.remove("unknown-error");

    document.querySelectorAll(".editor-line[data-error]").forEach(de => {
        de.removeAttribute("data-error");
    });

    if (ranges.length > 0) {
        ranges.forEach(range => highlights["ERROR"].delete(range));
        ranges = [];
    }
}

function showError(errordata) {
    const unknownError = typeof errordata !== "object";
    
    if (!unknownError && errordata.index < editor.value.length) {
        const { line } = errordata;
        const lineNode = editorHighlight.childNodes[line - 1];

        const range = new Range();
        const node = getNodeAtPosition(lineNode, errordata.column - 1);
        const endNode = getNodeAtPosition(lineNode, errordata.column - 1 + errordata.length, true);
        
        range.setStart(node.node.firstChild, errordata.column - 1 - node.i);
        range.setEnd(endNode.node.firstChild, errordata.column - 1 - endNode.i + errordata.length);

        ranges.push(range);
        highlights["ERROR"].add(range);

        lineNode.dataset.error = errordata.message;
    } else {
        error.innerText = unknownError ? errordata : errordata.message;
        error.style.display = "block";
        error.classList.add("unknown-error");

        setTimeout(() => {
            clearError();
        }, 4000);
    }
}

// let worker = new Worker("smi-worker.js", { type: "module" });
let worker = new SMIWorker();

runBtn.addEventListener("click", () => {
    if (editor.value.length == 0) return;
    output.innerHTML = "";
    runBtn.classList.add("hidden");
    stopBtn.classList.remove("hidden");

    worker.onmessage = (ev) => {
        if (ev.data.type !== "result" && ev.data.type !== "error")
            return;

        console.timeEnd("EvalCode");

        if (ev.data.type === "error") {
            showError(ev.data.error.data || "Unknown error");
        }

        for (const key in ev.data.memory) {
            const value = ev.data.memory[key];
            output.insertAdjacentHTML("beforeend",
                `<div class="row"><div>${key}</div><div>0x${value.toString(16)}</div><div>${value}</div></div>`
            );
        }

        stopBtn.classList.add("hidden");
        runBtn.classList.remove("hidden");
    };

    console.time("EvalCode");
    
    worker.postMessage({ type: "run", code: editor.value });
});

stopBtn.addEventListener("click", () => {
    worker.terminate();

    // worker = new Worker("smi-worker.js", { type: "module" });
    worker = new SMIWorker();

    console.timeEnd("EvalCode");

    stopBtn.classList.add("hidden");
    runBtn.classList.remove("hidden");
});

let smiDebugger = null;
const breakpoints = new Set();

function addBreakpoint(line) {
    breakpoints.add(line);
}

function removeBreakpoint(line) {
    breakpoints.delete(line);
}

function hasBreakpoint(line) {
    return breakpoints.has(line);
}

function toggleBreakpoint(line) {
    if (hasBreakpoint(line)) {
        removeBreakpoint(line);
    } else {
        addBreakpoint(line);
    }
}

function debuggerShowMemory() {
    output.innerHTML = "";

    const memory = smiDebugger.getMemoryKeys();

    for (const key of memory) {
        const value = smiDebugger.getMemoryValue(key);
        output.insertAdjacentHTML("beforeend",
            `<div class="row"><div>${key}</div><div>0x${value.toString(16)}</div><div>${value}</div></div>`
        );
    }

    editorHighlight.style.setProperty("--debugger-line-pos", `${smiDebugger.getNextLine() - 1}lh`);
}

const wait = (time) => {
    const { promise, resolve } = Promise.withResolvers();

    setTimeout(resolve, time);

    return promise;
}

async function debuggerRunUntilBreakpoint() {
    while (smiDebugger && smiDebugger.hasNext()) {
        if (smiDebugger.next() !== 0) {
            showError(SMI.getLastErrorData());
            
            return;
        }

        debuggerShowMemory();

        if (hasBreakpoint(smiDebugger.getNextLine()))
            break;

        await wait(10);
    }

    if (!smiDebugger)
        return;

    if (!smiDebugger.hasNext()) {
        document.getElementById("debug-stop").click();
    }
}

document.getElementById("debug-run").addEventListener("click", () => {
    debuggerRunUntilBreakpoint();
});

document.getElementById("debug").addEventListener("click", () => {
    document.getElementById("toolbar").classList.add("hidden");
    document.getElementById("debug-toolbar").classList.remove("hidden");
    editor.readOnly = true;

    smiDebugger = SMIDebugger();
    
    if (smiDebugger.load(editor.value) !== 0) {
        showError(SMI.getLastErrorData());

        return;
    }

    debuggerShowMemory();
    
    editorHighlight.classList.remove("hidden-after");
});

document.getElementById("debug-step").addEventListener("click", () => {
    if (!smiDebugger || !smiDebugger.hasNext())
        return;
    
    if (smiDebugger.next() !== 0) {
        showError(SMI.getLastErrorData());
        
        return;
    }

    debuggerShowMemory();

    if (!smiDebugger.hasNext()) {
        document.getElementById("debug-stop").click();
    }
});

document.getElementById("debug-stop").addEventListener("click", () => {
    document.getElementById("debug-toolbar").classList.add("hidden");
    document.getElementById("toolbar").classList.remove("hidden");

    editorHighlight.classList.add("hidden-after");

    editor.readOnly = false;

    smiDebugger.destroy();
    smiDebugger = null;
});

document.getElementById("share").addEventListener("click", () => {
    navigator.clipboard.writeText(document.location.href).then(() => {
        alert("URL copied to clipboard");
    }).catch(() => {
        alert("Failed to copy URL");
    });
});

document.getElementById("save").addEventListener("click", () => {
    document.querySelector(".save-file-error").textContent = "";
    saveFileDialog.classList.remove("hidden");
});

saveFileForm.addEventListener("submit", (ev) => {
    ev.preventDefault();

    const filename = saveFileForm.querySelector("input[name='filename']").value;
    const filetype = saveFileForm.querySelector(".sfd-option.selected").dataset.value;

    let blob = null;

    if (filetype === "MS") {
        const smiCompiler = SMICompiler();

        const result = smiCompiler.compile(editor.value);

        smiCompiler.destroy();

        if (!result) {
            const error = SMI.getLastErrorData();

            document.querySelector(".save-file-error").textContent = `Your file contains errors: ${error.message}`;
            showError(error);
            return;
        }

        blob = new Blob([result], { type: "application/octet-stream" });
    } else {
        blob = new Blob([editor.value], { type: "text/plain" });
    }

    const url = URL.createObjectURL(blob);

    const link = document.createElement("a");
    link.href = url;
    link.download = filename + "." + filetype;
    link.click();

    URL.revokeObjectURL(url);

    saveFileDialog.classList.add("hidden");
});

document.getElementById("open").addEventListener("click", () => {
    const input = document.createElement("input");
    
    input.type = "file";
    input.accept = ".txt,.MS";

    input.oninput = async () => {
        const file = input.files[0];

        if (!file)
            return;

        if (file.name.endsWith(".MS")) {
            const smiDecompiler = SMIDecompiler();

            const code = smiDecompiler.decompile(await file.bytes());

            smiDecompiler.destroy();

            setEditorContent(code, true);
        } else {
            setEditorContent(await file.text(), true);
        }
    }

    input.click();
});

document.addEventListener("keydown", (ev) => {
    if (ev.key === "F2") {
        ev.preventDefault();

        document.getElementById("debug-step").click();
    } else if (ev.key === "F9") {
        ev.preventDefault();

        if (runBtn.classList.contains("hidden"))
            return;

        runBtn.click();
    } else if (ev.ctrlKey) {
        if (ev.key === "o") {
            ev.preventDefault();
            document.getElementById("open").click();
        } else if (ev.key === "s") {
            ev.preventDefault();
            document.getElementById("save").click();
        }
    }
});

saveFileDialog.addEventListener("click", (ev) => {
    if (!saveFileForm.contains(ev.target)) {
        saveFileDialog.classList.add("hidden");
    }
});

document.querySelectorAll(".sfd-option").forEach(opt => {
    opt.addEventListener("click", (ev) => {
        document.querySelectorAll(".sfd-option").forEach(opt => {
            opt.classList.remove("selected");
        });

        ev.target.classList.add("selected");
    });
});
