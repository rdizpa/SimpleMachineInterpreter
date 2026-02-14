#ifndef _MS_DECOMPILER_H_
#define _MS_DECOMPILER_H_

#include <string>

#include "mstypes.h"

namespace smi::ms::decompiler {

typedef enum {
    DECOMPILER_OK = 0,
    DECOMPILER_ERR_INVALID_FILE,
    DECOMPILER_ERR_INVALID_OPCODE
} DecompilerError;

class MSDecompiler {
   private:
    unsigned int pos;
    const char* data;
    uint16_t numLabels;
    Line lines[DATASIZE];
    Label labels[DATASIZE];
    Instruction instructions[DATASIZE];

    void readData();
    std::string instructionToText(uint8_t op);

   public:
    int decompile(const char* data, unsigned int size, std::string& result);
};

}  // namespace smi::ms::decompiler

#endif
