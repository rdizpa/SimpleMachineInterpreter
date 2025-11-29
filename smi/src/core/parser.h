#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>

#include "ast.h"
#include "lexer.h"

namespace smi::parser {

using lexer::Token;
using lexer::Tokens;
using lexer::TokenType;

typedef enum {
    PARSER_OK = 0,
    PARSER_ERR_UNEXPECTED_TOKEN
} ParserError;

class Parser {
   private:
    int index;
    const std::vector<Token>* tokens;

    Node* parse(Token token, int& error);
    Node* parseOperand(Token token, int& error);
    Node* parseLabel(Token token, int& error);
    Node* parseInstr(Token token, int& error);
    bool tokenMatch(const Token& token, TokenType type, int& error);

   public:
    Parser() : index(0), tokens(nullptr) {}

    int parse(const Tokens& tokens, std::unique_ptr<Program>& program);
};

int parse(const Tokens& tokens, std::unique_ptr<Program>& program);

}  // namespace smi::parser

#endif
