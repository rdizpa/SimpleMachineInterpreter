#ifndef _MS_MS_H_
#define _MS_MS_H_

#include <stdint.h>

#define DATASIZE 0x80

namespace smi::ms {

typedef enum {
    NONE = 0,
    INST = 1,
    DATA = 2
} LineType;

typedef enum {
    ADD = 0,
    CMP = 1,
    MOV = 2,
    BEQ = 3
} OpCode;

struct Line {
    uint8_t type;
    uint16_t data;
    bool hasLabel;
    uint8_t undefinedLabels;
};

struct Instruction {
    uint8_t opcode;
    uint8_t op1;
    uint8_t op2;
};

struct Label {
    char name[7];
};

struct UndefinedLabel {
    char name1[7];
    char name2[7];
};

}  // namespace smi::ms

#endif
