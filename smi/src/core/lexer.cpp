#include "lexer.h"

#include <algorithm>
#include <sstream>

#include "smierror.h"

const std::vector<std::string> INSTRUCTIONS = {"MOV", "ADD", "CMP", "BEQ"};

namespace smi::lexer {

int tokenize(const std::string& code, Tokens& tokens) {
    int pos = 0;
    int line = 1;
    int lineStart = 0;

    while (pos < code.length()) {
        char ch = code[pos];
        std::string val;

        switch (ch) {
            case '\r':
            case '\t':
            case ' ':
                break;
            case '\n':
                line++;
                lineStart = pos + 1;
                break;
            case ',':
                tokens.push_back(TOKEN_NEW(TokenType::COMMA, ",", pos, line, pos - lineStart + 1));
                break;
            default:
                val = "";
                int valStartPos = pos;

                while (std::isalnum(code[pos])) {
                    val += code[pos];
                    pos++;
                }

                if (val.length() == 0) {
                    std::ostringstream buffer;
                    buffer << "Invalid token '" << ch << "'";

                    smi::error::setLastError(pos, line, pos - lineStart + 1, 1, buffer.str());
                    return LEXER_ERR_INVALID_TOKEN;
                }

                if (code[pos] == ':') {
                    tokens.push_back(TOKEN_NEW(TokenType::LABEL, val, valStartPos, line, valStartPos - lineStart + 1));
                } else if (std::find(INSTRUCTIONS.begin(), INSTRUCTIONS.end(), val) != INSTRUCTIONS.end()) {
                    tokens.push_back(TOKEN_NEW(TokenType::INSTR, val, valStartPos, line, valStartPos - lineStart + 1));
                    pos--;
                } else {
                    if (tokens.size() > 0 && tokens[tokens.size() - 1].type == TokenType::LABEL) {
                        tokens.push_back(
                            TOKEN_NEW(TokenType::LITERAL, val, valStartPos, line, valStartPos - lineStart + 1));
                    } else {
                        tokens.push_back(TOKEN_NEW(TokenType::OP, val, valStartPos, line, valStartPos - lineStart + 1));
                    }

                    pos--;
                }
        }

        pos++;
    }

    tokens.push_back(TOKEN_NEW(TokenType::_EOF, "", pos, line, pos - lineStart + 1));

    return LEXER_OK;
}

}  // namespace smi::lexer
