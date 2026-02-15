#include "decompiler.h"

#include <string.h>

#include <sstream>

namespace smi::ms::decompiler {

uint16_t MSDecompiler::read16(const char*& p) {
    uint8_t b2 = *p++;
    uint8_t b1 = *p++;

    return (b1 << 8) | b2;
}

int MSDecompiler::decompile(const char* data, unsigned int size, std::string& result) {
    this->pos = 0;
    this->data = data;

    uint8_t b2 = data[this->pos++];
    uint8_t b1 = data[this->pos++];

    this->numLabels = (b1 << 8) | b2;

    b2 = data[this->pos++];
    b1 = data[this->pos++];

    this->numUndefinedLabels = (b1 << 8) | b2;

    unsigned int minSize = 4 + DATASIZE * 2 + DATASIZE + this->numLabels * 7 + this->numLabels * 2 +
                           this->numUndefinedLabels * 7 + this->numUndefinedLabels * 2 + this->numUndefinedLabels * 2;

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

            std::string l1, l2;

            if (instructions[i].opcode != BEQ && (lines[i].undefinedLabels & 0b01) != 0) {
                l1 = undefinedLabels[i].name1;
            } else if (instructions[i].opcode == BEQ && (lines[i].undefinedLabels & 0b10) != 0) {
                l1 = undefinedLabels[i].name2;
            } else {
                l1 = labels[instructions[i].op1].name;
            }

            strCode << instructionText << " " << l1;

            if (instructions[i].opcode != BEQ) {
                if ((lines[i].undefinedLabels & 0b10) != 0) {
                    l2 = undefinedLabels[i].name2;
                } else {
                    l2 = labels[instructions[i].op2].name;
                }

                strCode << ", " << l2;
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
    const char* pUndefinedLabels = pLabelPos + this->numLabels * 2;
    const char* pUndefinedLabelPos = pUndefinedLabels + 7 * this->numUndefinedLabels;
    const char* pUndefinedLabelOp = pUndefinedLabelPos + this->numUndefinedLabels * 2;

    for (int i = 0; i < DATASIZE; i++) {
        uint8_t b2 = data[this->pos++];
        uint8_t b1 = data[this->pos++];
        uint16_t lineData = (b1 << 8) | b2;

        lines[i].type = types[i];
        lines[i].data = lineData;
        lines[i].hasLabel = false;
        lines[i].undefinedLabels = 0;

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
        uint16_t labelLinePos = read16(pLabelPos);

        lines[labelLinePos].hasLabel = true;

        memset(labels[labelLinePos].name, 0, 7);
        memcpy(labels[labelLinePos].name, pLabels, 6);
        pLabels += 7;
    }

    for (int i = 0; i < this->numUndefinedLabels; i++) {
        uint16_t labelInstructionPos = read16(pUndefinedLabelPos);
        uint16_t labelInstructionOp = read16(pUndefinedLabelOp);

        if (labelInstructionOp == 0) {
            lines[labelInstructionPos].undefinedLabels |= 0b01;

            memset(undefinedLabels[labelInstructionPos].name1, 0, 7);
            memcpy(undefinedLabels[labelInstructionPos].name1, pUndefinedLabels, 6);
        } else {
            lines[labelInstructionPos].undefinedLabels |= 0b10;

            memset(undefinedLabels[labelInstructionPos].name2, 0, 7);
            memcpy(undefinedLabels[labelInstructionPos].name2, pUndefinedLabels, 6);
        }

        pUndefinedLabels += 7;
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
