#include <fstream>
#include <iostream>
#include <sstream>

#include "core/interpreter.h"
#include "core/ms/decompiler.h"

int main(int argc, char* argv[]) {
    smi::interpreter::Interpreter interp;

    if (argc < 2) {
        std::cout << "Usage: smi [file]" << std::endl;
        return 0;
    }

    std::string filename = argv[1];

    try {
        std::ifstream ifs(filename);
        std::ostringstream buffer;

        buffer << ifs.rdbuf();

        ifs.close();

        std::string code = buffer.str();

        if (filename.find(".MS") != std::string::npos) {
            smi::ms::decompiler::MSDecompiler dec;
            std::string result;

            if (dec.decompile(code.c_str(), code.length(), result) != smi::ms::decompiler::DECOMPILER_OK) {
                throw std::runtime_error("Invalid file");
            }

            std::cout << result << std::endl << std::endl;
            code = result;
        }

        if (interp.eval(code) != smi::interpreter::INTERPRETER_OK) {
            std::cout << "Error: " << smi::error::getLastErrorMessage() << std::endl;

            int pos = smi::error::getLastError().index;
            int lineStart = pos - smi::error::getLastError().column + 1;
            int lineEnd = pos;
            while (lineEnd < code.length() - 1 && code[lineEnd] != '\n') lineEnd++;

            if (lineEnd == code.length() - 1) lineEnd++;

            std::cout << code.substr(lineStart, lineEnd - lineStart) << std::endl;
            std::cout << std::string(pos - lineStart, '-') << std::string(smi::error::getLastError().length, '^')
                      << std::endl;
        }

        for (auto key : interp.getMemoryKeys()) {
            std::cout << key << "\t" << std::hex << interp.getMemoryValue(key) << std::endl;
        }
    } catch (std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
