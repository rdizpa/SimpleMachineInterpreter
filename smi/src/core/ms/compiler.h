#ifndef _MS_COMPILER_H_
#define _MS_COMPILER_H_

#include <map>
#include <string>

#include "mstypes.h"

namespace smi::ms::compiler {

typedef enum {
    COMPILER_OK = 0,
    COMPILER_ERR_LEXER,
    COMPILER_ERR_LABEL_TOO_LONG,
    COMPILER_ERR_UNDEFINED_LABEL,
    COMPILER_ERR_FILE_TOO_LONG
} CompilerError;

class MSCompiler {
   private:
    unsigned int pos;
    const char* data;
    uint16_t numLabels;
    Line lines[DATASIZE];
    Label labels[DATASIZE];
    Instruction instructions[DATASIZE];
    std::map<std::string, uint8_t> labelToPos;

    uint8_t textToInstruction(const std::string& text);

   public:
    int compile(const std::string& code, std::string& result);
};

}  // namespace smi::ms::compiler

#endif
