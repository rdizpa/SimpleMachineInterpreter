#pragma once

#include <fstream>
#include <sstream>

std::string readFile(const char* path) {
    std::ifstream ifs(path, std::ios::binary);
    std::ostringstream buffer;
    buffer << ifs.rdbuf();
    ifs.close();
    return buffer.str();
}
