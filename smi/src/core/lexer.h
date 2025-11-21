#ifndef _LEXER_H_
#define _LEXER_H_

#include <exception>
#include <string>
#include <vector>

#define TOKEN_NEW(typ, val, idx, ln, col) (Token{.type = typ, .value = val, .index = idx, .line = ln, .column = col})

namespace smi::lexer {

typedef enum {
    LEXER_OK = 0,
    LEXER_ERR_INVALID_TOKEN
} LexerError;

enum class TokenType {
    LABEL,
    INSTR,
    OP,
    LITERAL,
    COMMA
};

struct Token {
    TokenType type;
    std::string value;
    int index;
    int line;
    int column;
};

typedef std::vector<Token> Tokens;

int tokenize(const std::string& code, Tokens& tokens);

}  // namespace smi::lexer

#endif
