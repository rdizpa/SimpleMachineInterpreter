import "./style.css"

import SMIWorker from "./smi-worker.js?worker";

const editor = document.getElementById("editor");
const output = document.querySelector(".output .content");
const runBtn = document.getElementById("run");
const stopBtn = document.getElementById("stop");
const error = document.getElementById("error");
const lineNumberColumn = document.querySelector(".line-number-column");

window.addEventListener("load", () => {
    const params = new URLSearchParams(document.location.href.split("?")[1]);

    if (params.has("d")) {
        setEditorContent(window.atob(params.get("d")));
    }
});

const UNDEFINED_LABEL_RE = /(Undefined label '([^']*)') at position (\d+) \(line: (\d+), column: (\d+)\)/;
const INVALID_TOKEN_RE = /(Invalid token '(.)') at position (\d+) \(line: (\d+), column: (\d+)\)/;

const INSTRUCTIONS = ["MOV", "ADD", "CMP", "BEQ"];
const chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789:";
const highlights = {};

highlights["INSTRUCTION"] = new Highlight();
highlights["NUMBER"] = new Highlight();
highlights["LABEL"] = new Highlight();
highlights["VAR"] = new Highlight();
highlights["ERROR"] = new Highlight();

CSS.highlights.set("keyword", highlights["INSTRUCTION"]);
CSS.highlights.set("number", highlights["NUMBER"]);
CSS.highlights.set("_label", highlights["LABEL"]);
CSS.highlights.set("_var", highlights["VAR"]);
CSS.highlights.set("error", highlights["ERROR"]);

/**
 * 
 * @param {string} line 
 * @returns 
 */
const normalize = (line) => {
    const matches = line.matchAll(new RegExp(`\\b(${INSTRUCTIONS.join("|")})\\b`, "gdi"));

    for (const m of matches) {
        line = line.substring(0, m.indices[1][0]) + m[1].toUpperCase() + line.substring(m.indices[1][1]);
    }

    for (const m of editor.textContent.matchAll(/\b([A-Z0-9a-z]+)\b:/gd)) {
        for (const ml of line.matchAll(new RegExp(`\\b(${m[1]})\\b([^:]|$)`, "gdi")))
            line = line.substring(0, ml.indices[1][0]) + m[1] + line.substring(ml.indices[1][1]);
    }

    return line;
};

const getNodeAtPosition = (pos) => {
    let i = 0;
    let lastI = 0;
    let nodeIndex = 0;

    while (i <= pos && nodeIndex < editor.childNodes.length) {
        lastI = i;
        i += editor.childNodes[nodeIndex].textContent.length;

        if (i > pos)
            break;

        nodeIndex++;
    }

    if (nodeIndex >= editor.childNodes.length)
        return null;
    
    return {
        node: editor.childNodes[nodeIndex],
        i: lastI
    };
};

const colorize = () => {
    const code = editor.textContent;
    const matches = code.matchAll(new RegExp(`\\b(${INSTRUCTIONS.join("|")})\\b`, "g"));
    const res = [];

    matches.forEach(m => {
        res.push({
            type: "INSTRUCTION",
            offset: m.index,
            length : m[0].length,
            start: m.index,
            end: m.index + m[0].length
        })
    });

    code.matchAll(/\b:[ \t]*([0-9A-Fa-f]+)\b/gd).forEach(m => {
        res.push({
            type: "NUMBER",
            offset: m.indices[1][0],
            length : m[1].length,
            start: m.indices[1][0],
            end: m.indices[1][1]
        });
    });

    code.matchAll(/\b([A-Z0-9a-z]+)\b:/gd).forEach(m => {
        res.push({
            type: "LABEL",
            offset: m.indices[1][0],
            length : m[1].length,
            start: m.indices[1][0],
            end: m.indices[1][1]
        });

        code.matchAll(new RegExp(`\\b(${m[1]})\\b([^:]|$)`, "gd")).forEach(l => {
            res.push({
                type: "VAR",
                offset: l.indices[1][0],
                length : l[1].length,
                start: l.indices[1][0],
                end: l.indices[1][1]
            });
        });

    });

    highlights.INSTRUCTION.clear();
    highlights.NUMBER.clear();
    highlights.LABEL.clear();
    highlights.VAR.clear();

    for (const tk of res) {
        const startNode = getNodeAtPosition(tk.start);
        const posInNode = tk.start - startNode.i;
        const range = new Range();

        range.setStart(startNode.node, posInNode);
        range.setEnd(startNode.node, posInNode + tk.length);

        highlights[tk.type].add(range);
    }
}

const updateLineNumberColumn = () => {
    const lines = editor.textContent.split("\n").length - (editor.textContent.endsWith("\n") ? 1 : 0);
    const currentLines = parseInt(lineNumberColumn.dataset.lines) || 1;
    
    if (lines == currentLines)
        return;

    let str = "";

    for (let i = 0; i < lines; i++)
        str += `${i + 1}\n`;

    lineNumberColumn.innerText = str;
    lineNumberColumn.dataset.lines = lines;
};

const setEditorContent = (content, updateURL = false) => {
    editor.textContent = content;
    updateLineNumberColumn();
    colorize();

    if (updateURL)
        window.history.replaceState({}, "", "/?d=" + window.btoa(editor.textContent));
};

editor.addEventListener("scroll", (ev) => {
    lineNumberColumn.scrollTop = editor.scrollTop;
});

editor.addEventListener("input", (ev) => {
    const selection = window.getSelection();

    if (selection.anchorNode instanceof Text) {
        const offset = selection.anchorOffset;
        selection.anchorNode.textContent = normalize(selection.anchorNode.textContent);
        selection.getRangeAt(0).setStart(selection.anchorNode, offset);
        selection.getRangeAt(0).setEnd(selection.anchorNode, offset);
    }

    updateLineNumberColumn();
    colorize();

    window.history.replaceState({}, "", "/?d=" + window.btoa(editor.textContent));
});

function showError(errordata) {
    const unknownError = typeof errordata !== "object";
    error.innerText = unknownError ? errordata : errordata.message;
    error.style.display = "block";

    const ranges = [];
    
    if (!unknownError) {
        const range = new Range();
        const node = getNodeAtPosition(errordata.index);

        range.setStart(node.node, errordata.index - node.i);
        range.setEnd(node.node, errordata.index - node.i + errordata.length);

        ranges.push(range);
        highlights["ERROR"].add(range);

        error.innerText = errordata.message;
    } else {
        error.classList.add("unknown-error");
    }

    if (ranges.length > 0) {
        const lineRange = ranges[0].cloneRange();
        let rangeEnd = lineRange.endOffset + 1;
        
        while (rangeEnd < lineRange.endContainer.textContent.length
            && lineRange.endContainer.textContent.at(rangeEnd) != "\n") {
            rangeEnd++;
        }

        lineRange.setEnd(lineRange.endContainer, rangeEnd - 1);
        const rect = lineRange.getBoundingClientRect();
        error.style.transform = `translate(${rect.left + rect.width + 20}px,${rect.top}px)`;
    }

    setTimeout(() => {
        error.style.display = "none";
        error.style.transform = "";
        error.classList.remove("unknown-error");
        (ranges.length > 0) && ranges.forEach(range => highlights["ERROR"].delete(range));
    }, 4000);
}

// let worker = new Worker("smi-worker.js", { type: "module" });
let worker = new SMIWorker();

runBtn.addEventListener("click", () => {
    if (editor.textContent.length == 0) return;
    output.innerHTML = "";
    runBtn.disabled = true;
    stopBtn.disabled = false;

    // const smi = SMI();

    // smi.eval(editor.textContent);

    // const keys = smi.getMemoryKeys();

    worker.onmessage = (ev) => {
        if (ev.data.type !== "result" && ev.data.type !== "error")
            return;

        console.timeEnd("EvalCode");

        if (ev.data.type === "error") {
            showError(ev.data.error.data || "Unknown error");
        }

        for (const key in ev.data.memory) {
            // const value = smi.getMemoryValue(key);
            const value = ev.data.memory[key];
            output.insertAdjacentHTML("beforeend",
                `<div class="row"><div>${key}</div><div>0x${value.toString(16)}</div><div>${value}</div></div>`
            );
        }

        stopBtn.disabled = true;
        runBtn.disabled = false;
    };

    console.time("EvalCode");
    
    worker.postMessage({ type: "run", code: editor.textContent });

    // smi.destroy();
});

stopBtn.addEventListener("click", () => {
    worker.terminate();

    // worker = new Worker("smi-worker.js", { type: "module" });
    worker = new SMIWorker();

    console.timeEnd("EvalCode");

    stopBtn.disabled = true;
    runBtn.disabled = false;
});

document.getElementById("share").addEventListener("click", () => {
    navigator.clipboard.writeText(document.location.href).then(() => {
        alert("URL copied to clipboard");
    }).catch(() => {
        alert("Failed to copy URL");
    });
});

document.getElementById("save").addEventListener("click", () => {
    const blob = new Blob([editor.textContent], { type: "text/plain" });
    const url = URL.createObjectURL(blob);

    const link = document.createElement("a");
    link.href = url;
    link.download = "SMI_Work.txt";
    link.click();

    URL.revokeObjectURL(url);
});

document.getElementById("open").addEventListener("click", () => {
    const input = document.createElement("input");
    
    input.type = "file";
    input.accept = ".txt";

    input.oninput = () => {
        input.files[0].text().then((code) => {
            setEditorContent(code, true);
        });
    }

    input.click();
});

document.addEventListener("keydown", (ev) => {
    if (ev.key === "F9") {
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
