import { execSync } from "node:child_process";
import { copyFileSync, existsSync, mkdirSync } from "node:fs";
import path from "node:path";

const proyectDir = "../";
const smiDir = path.join(proyectDir, "smi/");
const buildDir = path.join(proyectDir, "build-wasm/");

if (!existsSync(buildDir)) {
    mkdirSync(buildDir);
}

execSync(`emcmake cmake ${smiDir} -B ${buildDir} -DCMAKE_BUILD_TYPE=Release`);
execSync(`emmake make -C ${buildDir}`);

if (!existsSync("./public")) {
    mkdirSync("./public");
}

copyFileSync(path.join(buildDir, "smiwasmout.wasm"), "./public/smiwasmout.wasm");
copyFileSync(path.join(buildDir, "smiwasmout.js"), "./src/smiwasmout.js");
