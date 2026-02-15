#include "compiler.h"

#include <string.h>

#include "core/lexer.h"
#include "core/parser.h"
#include "core/smierror.h"

namespace smi::ms::compiler {

using lexer::TokenType;

int MSCompiler::compile(const std::string& code, std::string& result) {
    lexer::Tokens tokens;

    if (lexer::tokenize(code, tokens) != lexer::LEXER_OK) {
        return COMPILER_ERR_LEXER;
    }

    this->pos = 0;
    this->numLabels = 0;
    unsigned int tk = 0;

    memset(this->lines, 0, sizeof(Line) * DATASIZE);
    memset(this->labels, 0, sizeof(Label) * DATASIZE);
    memset(this->instructions, 0, sizeof(Instruction) * DATASIZE);

    while (tk < tokens.size() && tokens[tk].type != TokenType::_EOF) {
        const lexer::Token& token = tokens[tk++];

        switch (token.type) {
            case TokenType::LABEL:
                if (this->pos >= DATASIZE) {
                    lexer::Token& token = tokens[tk - 1];
                    smi::error::setLastError(token.index, token.line, token.column, token.value.length(),
                                             "File too long");

                    return COMPILER_ERR_FILE_TOO_LONG;
                }

                this->lines[this->pos].hasLabel = true;

                if (token.value.length() > 6) {
                    smi::error::setLastError(token.index, token.line, token.column, token.value.length(),
                                             "Label too long");

                    return COMPILER_ERR_LABEL_TOO_LONG;
                }

                strcpy(this->labels[this->pos].name, token.value.c_str());
                labelToPos[token.value] = this->pos;
                this->numLabels++;
                break;

            case TokenType::INSTR:
            case TokenType::LITERAL:
                if (this->pos >= DATASIZE) {
                    lexer::Token& token = tokens[tk - 1];
                    smi::error::setLastError(token.index, token.line, token.column, token.value.length(),
                                             "File too long");

                    return COMPILER_ERR_FILE_TOO_LONG;
                }

                this->pos++;
                break;
            default:
                break;
        }
    }

    this->pos = 0;
    tk = 0;

    while (tk < tokens.size() && tokens[tk].type != TokenType::_EOF) {
        lexer::Token& token = tokens[tk++];

        switch (token.type) {
            case TokenType::LITERAL:
                this->lines[this->pos].type = LineType::DATA;
                this->lines[this->pos].data = std::stoi(token.value, NULL, 16);
                this->pos++;
                break;

            case TokenType::INSTR: {
                uint8_t opcode = textToInstruction(token.value);

                token = tokens[tk++];

                if (labelToPos.find(token.value) == labelToPos.end()) {
                    smi::error::setLastError(token.index, token.line, token.column, token.value.length(),
                                             "Undefined label");

                    return COMPILER_ERR_LABEL_TOO_LONG;
                }

                uint8_t op1 = labelToPos[token.value];
                uint8_t op2 = op1;

                if (opcode != BEQ) {
                    tk++;
                    token = tokens[tk++];

                    if (labelToPos.find(token.value) == labelToPos.end()) {
                        smi::error::setLastError(token.index, token.line, token.column, token.value.length(),
                                                 "Undefined label");

                        return COMPILER_ERR_LABEL_TOO_LONG;
                    }

                    op2 = labelToPos[token.value];
                } else {
                    op1 = 0x00;
                }

                this->lines[this->pos].type = LineType::INST;
                this->lines[this->pos].data = (opcode << 14) | (op1 << 7) | op2;

                this->pos++;
                break;
            }
            default:
                break;
        }
    }

    unsigned int fileSize = 4 + DATASIZE * 2 + DATASIZE + this->numLabels * 7 + this->numLabels * 2;
    char* file = new char[fileSize];
    char* data = file + 4;
    char* dataType = data + DATASIZE * 2;
    char* pLabels = dataType + DATASIZE;
    char* pLabelPos = pLabels + this->numLabels * 7;

    uint8_t b1 = (this->numLabels & 0xFF00) >> 8;
    uint8_t b2 = this->numLabels & 0x00FF;

    file[0] = b2;
    file[1] = b1;
    file[2] = 0;
    file[3] = 0;

    for (int i = 0; i < DATASIZE; i++) {
        dataType[i] = lines[i].type;

        if (lines[i].hasLabel) {
            strcpy(pLabels, labels[i].name);
            pLabels += 7;

            uint8_t b1 = (i >> 8) & 0xFF;
            uint8_t b2 = i & 0xFF;

            *pLabelPos++ = b2;
            *pLabelPos++ = b1;
        }

        uint8_t b1 = (lines[i].data >> 8) & 0xFF;
        uint8_t b2 = lines[i].data & 0xFF;

        data[i * 2] = b2;
        data[i * 2 + 1] = b1;
    }

    std::string fileOut(file, fileSize);

    delete[] file;

    result = fileOut;
    return COMPILER_OK;
}

uint8_t MSCompiler::textToInstruction(const std::string& text) {
    if (text == "ADD") {
        return ADD;
    } else if (text == "MOV") {
        return MOV;
    } else if (text == "CMP") {
        return CMP;
    } else if (text == "BEQ") {
        return BEQ;
    } else {
        return -1;
    }
}

}  // namespace smi::ms::compiler
