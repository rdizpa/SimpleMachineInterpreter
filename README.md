# Simple Machine Interpreter

SMI (Simple Machine Interpreter) is an interpreter for a simple **4-instructions based architecture**.

Supported instructions are MOV, ADD, CMP and BEQ.

```arm
MOV from, to
ADD from, to
CMP l1, l2
BEQ label
```

Where _from_ , _to_, _l1_, _l2_ and _label_ are memory labels.

Example:
```arm
ONE: 1
FIVE: 5
INDEX: 0

MOV ONE, INDEX

LOOP:
CMP INDEX, FIVE
BEQ END
ADD UNO, INDEX
CMP UNO, UNO
BEQ LOOP

END:
```

For more examples, see examples/

___
There are two different targets on the proyect:
 - CLI Application. See [Build CLI](#build-cli)
 - Web Application. See [Build Web](#build-web)

You can also [build the WASM](#building-wasm)

## Build CLI

```bash
mkdir build-cli && cd build-cli
cmake ../smi
make
```

## Build Web

```bash
cd web/
pnpm configure
pnpm build
```

## Build WASM

```bash
mkdir build-wasm
cd build-wasm
emcmake cmake ../smi
emmake make
```
