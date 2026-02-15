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
    uint16_t numUndefinedLabels;
    Line lines[DATASIZE];
    Label labels[DATASIZE];
    UndefinedLabel undefinedLabels[DATASIZE];
    Instruction instructions[DATASIZE];

    uint16_t read16(const char*& p);

    void readData();
    std::string instructionToText(uint8_t op);

   public:
    int decompile(const char* data, unsigned int size, std::string& result);
};

}  // namespace smi::ms::decompiler

#endif
