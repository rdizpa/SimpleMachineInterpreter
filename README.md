# Simple Machine Interpreter

SMI (Simple Machine Interpreter) is an interpreter for a simple **4-instructions based architecture**.

The supported instructions are MOV, ADD, CMP and BEQ.

```arm
MOV from, to
ADD from, to
CMP l1, l2
BEQ label
```

Where _from_, _to_, _l1_, _l2_ and _label_ are memory labels.

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

For more examples, see the `examples` folder.

___
There are two different targets on the proyect:
 - CLI Application. See [Build CLI](#build-cli).
 - Web Application. See [Build Web](#build-web) (~~requires [building the WASM](#build-wasm) first~~).

# Building instructions

Clone the project:
```bash
git clone https://github.com/rdizpa/SimpleMachineInterpreter.git
```

## Build CLI

```bash
mkdir build-cli
cd build-cli

# Build on Unix-based systems:
cmake ../smi
make

# Build with Ninja (either in Unix-based or Windows):
cmake ../smi -G "Ninja"
ninja

# Build with Visual Studio:
cmake ../smi -G "Visual Studio 17 2022"
cmake --build . --config Release
```

> [!NOTE]
> If you're not going to build the web version ignore the rest of the building steps.
> You should be good to go with the `smi` executable.

## Emscripten installation

Follow [this link](https://emscripten.org/docs/getting_started/downloads.html) to go to the official `emscripten` installation guide.

Here's a quick cross-platform guide of the steps required to do the recommended install:

#### Run once commands:

```bash
# Create or go to a folder in your system where you are going to install the
# emscripten's emsdk folder.
cd <SOMEWHERE>

# Get the emsdk repository folder.
git clone https://github.com/emscripten-core/emsdk.git

# Enter that directory.
cd emsdk

# Fetch the latest version of the emsdk (not needed the first time you clone).
git pull

# Download and install the latest SDK tools.
emsdk install latest # prepend with `.\` (Windows) or `./` (Linux)

# Make the "latest" SDK "active" for the current user (writes .emscripten file).
# Add --system flag in a privileged terminal to this command so that the emscripten
# environment variables are set permanently.

emsdk activate latest # prepend with `.\` (Windows) or `./` (Linux)
```

> [!WARNING]
> The use of (`sudo`) `emsdk activate latest --system` alone doesn't remove the need of
> running `emsdk_env` on new terminals.

#### Run on new terminals:

```bash
# Activate PATH and other environment variables in the current terminal.
emsdk_env # prepend with `.\` (Windows) or `./` (Linux)
```

## Build WASM

> [!NOTE]
> You can skip these WASM building steps because `pnpm configure` or `node configure.js`
> on [Build Web](#build-web) already do this automatically. Go ahead to [build web](#build-web).

First go to the main project's folder.

```bash
mkdir build-wasm
cd build-wasm
emcmake cmake ../smi
emmake make

cd ..

# Create the web/public directory if it doesn't exist.
mkdir web/public

# Copy the build-wasm/smiwasmout.wasm to web/public/smiwasmout.wasm
# Linux:
cp build-wasm/smiwasmout.wasm web/public/smiwasmout.wasm
# Windows:
# copy build-wasm\smiwasmout.wasm web\public\smiwasmout.wasm

# Copy the build-wasm/smiwasmout.js to web/src/smiwasmout.js
# Linux:
cp build-wasm/smiwasmout.js web/src/smiwasmout.js
# Windows:
# copy build-wasm\smiwasmout.js web\src\smiwasmout.js
```

## Build Web

```bash
cd web

npm i
npm run configure
# npm run dev
npm run build
```
