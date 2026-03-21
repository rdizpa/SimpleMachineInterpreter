#include "vm.h"

#include <string.h>

namespace smi::ms {

uint8_t VM::read(unsigned char* p, unsigned int& pos) {
    return p[pos++];
}

uint16_t VM::read16(unsigned char* p, unsigned int& pos) {
    uint8_t b2 = p[pos++];
    uint8_t b1 = p[pos++];

    return (b1 << 8) | b2;
}

void VM::loadMS(unsigned char* ms) {
    unsigned int pos = 0;

    uint16_t labelCount = read16(ms, pos);
    uint16_t undefindLabelCount = read16(ms, pos);

    for (int i = 0; i < DATASIZE; i++) {
        this->memory[i] = read16(ms, pos);
    }

    unsigned char* pLineType = ms + pos;

    pos += DATASIZE;

    unsigned char* pLabelPos = ms + pos + 7 * labelCount;
    unsigned int pLabelPosIndex = 0;

    for (uint16_t l = 0; l < labelCount; l++) {
        char label[7] = {0};
        memcpy(label, ms + pos, 6);
        std::string labelStr = label;

        this->labelKeys.push_back(labelStr);
        uint16_t labelPos = read16(pLabelPos, pLabelPosIndex);
        this->labels[labelStr] = labelPos;

        if (pLineType[labelPos] == DATA) this->dataLabelKeys.push_back(labelStr);

        pos += 7;
    }
}

void VM::loadMemory(unsigned char* memory) {
    memcpy(this->memory, memory, DATASIZE * 2);
    this->pc = 0;
}

int VM::executeNext() {
    if (this->pc >= DATASIZE) return 1;

    this->ir = memory[this->pc++];

    uint8_t opcode = (this->ir >> 14) & 0x03;
    uint8_t op1 = (this->ir >> 7) & 0x7F;
    uint8_t op2 = this->ir & 0x7F;

    switch (opcode) {
        case CMP:
            this->zf = (memory[op1] == memory[op2]);
            break;
        case ADD:
            memory[op2] += memory[op1];
            break;
        case MOV:
            memory[op2] = memory[op1];
            break;
        case BEQ:
            if (this->zf) {
                this->pc = op2;
            }

            break;
    }

    return 0;
}

}  // namespace smi::ms
