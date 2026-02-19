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

const getNodeAtPosition = (pos, endNode = false) => {
    let i = 0;
    let lastI = 0;
    let nodeIndex = 0;

    while (i <= pos && nodeIndex < editorHighlight.childNodes.length) {
        lastI = i;
        i += editorHighlight.childNodes[nodeIndex].textContent.length;

        if (i > pos)
            break;

        if (endNode && i == pos)
            break;

        nodeIndex++;
    }

    if (nodeIndex >= editorHighlight.childNodes.length)
        return null;
    
    return {
        node: editorHighlight.childNodes[nodeIndex],
        i: lastI
    };
};

const colorize = () => {
    const code = editor.value;
    const matches = code.matchAll(new RegExp(`\\b(${INSTRUCTIONS.join("|")})\\b`, "g"));
    const res = [];

    matches.forEach(m => {
        res.push({
            type: "instruction",
            offset: m.index,
            length : m[0].length,
            start: m.index,
            end: m.index + m[0].length
        })
    });

    code.matchAll(/\b:[ \t]*((?:0[xX])?[0-9A-Fa-f]+)\b/gd).forEach(m => {
        res.push({
            type: "number",
            offset: m.indices[1][0],
            length : m[1].length,
            start: m.indices[1][0],
            end: m.indices[1][1]
        });
    });

    code.matchAll(/\b([A-Z0-9a-z]+)\b:/gd).forEach(m => {
        res.push({
            type: "label",
            offset: m.indices[1][0],
            length : m[1].length,
            start: m.indices[1][0],
            end: m.indices[1][1]
        });

        code.matchAll(new RegExp(`\\b(${m[1]})\\b([^:]|$)`, "gd")).forEach(l => {
            res.push({
                type: "var",
                offset: l.indices[1][0],
                length : l[1].length,
                start: l.indices[1][0],
                end: l.indices[1][1]
            });
        });

    });

    code.matchAll(/(;[^\n]*)/gd).forEach(m => {
        res.push({
            type: "comment",
            offset: m.indices[1][0],
            length : m[1].length,
            start: m.indices[1][0],
            end: m.indices[1][1]
        });
    });

    code.matchAll(/(\n)/gd).forEach(m => {
        res.push({
            type: "newline",
            offset: m.indices[1][0],
            length : m[1].length,
            start: m.indices[1][0],
            end: m.indices[1][1]
        });
    });

    res.sort((a, b) => a.start - b.start);

    let codeResult = "";
    let lastIndex = 0;

    for (const tk of res) {
        if (tk.start < lastIndex)
            continue;

        codeResult += `<span>${code.substring(lastIndex, tk.start)}</span>`;

        codeResult += `<span class="${tk.type}">${code.substring(tk.start, tk.end)}</span>`;

        lastIndex = tk.end;
    }

    codeResult += `<span>${code.substring(lastIndex)}</span><br/>`;

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

    if (ranges.length > 0) {
        ranges.forEach(range => highlights["ERROR"].delete(range));
        ranges = [];
    }
}

function showError(errordata) {
    const unknownError = typeof errordata !== "object";
    error.innerText = unknownError ? errordata : errordata.message;
    error.style.display = "block";
    
    if (!unknownError && errordata.index < editor.value.length) {
        const range = new Range();
        const node = getNodeAtPosition(errordata.index);
        const endNode = getNodeAtPosition(errordata.index + errordata.length, true);

        range.setStart(node.node.firstChild, errordata.index - node.i);
        range.setEnd(endNode.node.firstChild, errordata.index - endNode.i + errordata.length);

        ranges.push(range);
        highlights["ERROR"].add(range);

        error.innerText = errordata.message;
    } else {
        error.classList.add("unknown-error");

        setTimeout(() => {
            clearError();
        }, 4000);
    }

    if (ranges.length > 0) {
        const errorRange = ranges[0].cloneRange();
        let lineEndContainer = errorRange.endContainer.parentElement;

        while (lineEndContainer.nextSibling != null && !lineEndContainer.textContent.includes("\n")) {
            lineEndContainer = lineEndContainer.nextSibling
        }

        const rect = lineEndContainer.getBoundingClientRect();
        error.style.transform = `translate(${rect.left + rect.width + 20}px,${rect.top}px)`;
    }
}

// let worker = new Worker("smi-worker.js", { type: "module" });
let worker = new SMIWorker();

runBtn.addEventListener("click", () => {
    if (editor.value.length == 0) return;
    output.innerHTML = "";
    runBtn.disabled = true;
    stopBtn.disabled = false;

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

        stopBtn.disabled = true;
        runBtn.disabled = false;
    };

    console.time("EvalCode");
    
    worker.postMessage({ type: "run", code: editor.value });
});

stopBtn.addEventListener("click", () => {
    worker.terminate();

    // worker = new Worker("smi-worker.js", { type: "module" });
    worker = new SMIWorker();

    console.timeEnd("EvalCode");

    stopBtn.disabled = true;
    runBtn.disabled = false;
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
    while (smiDebugger.hasNext()) {
        if (smiDebugger.next() !== 0) {
            showError(SMI.getLastErrorData());
            
            return;
        }

        debuggerShowMemory();

        if (hasBreakpoint(smiDebugger.getNextLine()))
            break;

        await wait(10);
    }

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
    editor.disabled = true;

    smiDebugger = SMIDebugger();
    
    if (smiDebugger.load(editor.value) !== 0) {
        showError(SMI.getLastErrorData());

        return;
    }

    debuggerShowMemory();
    
    editorHighlight.classList.remove("hidden-after");
});

document.getElementById("debug-step").addEventListener("click", () => {
    if (!smiDebugger.hasNext())
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

    editor.disabled = false;

    smiDebugger.destroy();
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
