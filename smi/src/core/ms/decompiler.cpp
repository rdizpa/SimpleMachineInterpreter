#include "decompiler.h"

#include <string.h>

#include <sstream>

namespace smi::ms::decompiler {

int MSDecompiler::decompile(const char* data, unsigned int size, std::string& result) {
    this->pos = 0;
    this->data = data;

    uint8_t b2 = data[this->pos++];
    uint8_t b1 = data[this->pos++];

    this->numLabels = (b1 << 8) | b2;
    this->pos += 2;

    unsigned int minSize = 4 + DATASIZE * 2 + DATASIZE + this->numLabels * 7 + this->numLabels * 2;

    if (size < minSize) return DECOMPILER_ERR_INVALID_FILE;

    this->readData();

    std::ostringstream strCode;

    for (int i = 0; i < DATASIZE; i++) {
        if (lines[i].hasLabel) {
            if (lines[i].type != DATA) strCode << std::endl;

            strCode << labels[i].name << ":";

            if (lines[i].type != DATA)
                strCode << std::endl;
            else
                strCode << " ";
        }

        if (lines[i].type == INST) {
            std::string instructionText = instructionToText(instructions[i].opcode);

            if (instructionText.empty()) return DECOMPILER_ERR_INVALID_OPCODE;

            strCode << instructionText << " " << labels[instructions[i].op1].name;

            if (instructions[i].opcode != BEQ) {
                strCode << ", " << labels[instructions[i].op2].name;
            }

            strCode << std::endl;
        } else if (lines[i].type == DATA) {
            strCode << std::hex << lines[i].data << std::endl;
        }
    }

    result = strCode.str();

    return DECOMPILER_OK;
}

void MSDecompiler::readData() {
    const char* const types = data + 4 + DATASIZE * 2;
    const char* pLabels = types + DATASIZE;
    const char* pLabelPos = pLabels + 7 * this->numLabels;

    for (int i = 0; i < DATASIZE; i++) {
        uint8_t b2 = data[this->pos++];
        uint8_t b1 = data[this->pos++];
        uint16_t lineData = (b1 << 8) | b2;

        lines[i].type = types[i];
        lines[i].data = lineData;
        lines[i].hasLabel = false;

        if (lines[i].type == INST) {
            uint16_t instruction = lines[i].data;

            uint8_t opcode = (instruction & 0xC000) >> 14;
            uint8_t op1 = (instruction & 0x3F80) >> 7;
            uint8_t op2 = instruction & 0x7F;

            if (opcode == BEQ) {
                op1 = op2;
            }

            instructions[i] = {.opcode = opcode, .op1 = op1, .op2 = op2};
        }
    }

    for (int i = 0; i < this->numLabels; i++) {
        uint8_t b2 = *pLabelPos++;
        uint8_t b1 = *pLabelPos++;
        uint16_t labelLinePos = (b1 << 8) | b2;

        lines[labelLinePos].hasLabel = true;

        memset(labels[labelLinePos].name, 0, 7);
        memcpy(labels[labelLinePos].name, pLabels, 6);
        pLabels += 7;
    }
}

std::string MSDecompiler::instructionToText(uint8_t op) {
    switch (op) {
        case ADD:
            return "ADD";
        case MOV:
            return "MOV";
        case CMP:
            return "CMP";
        case BEQ:
            return "BEQ";
        default:
            return "";
    }
}

}  // namespace smi::ms::decompiler
