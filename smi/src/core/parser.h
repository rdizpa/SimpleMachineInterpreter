#ifndef _PARSER_H_
#define _PARSER_H_

#include <memory>

#include "ast.h"
#include "lexer.h"

namespace smi::parser {

using lexer::Token;
using lexer::Tokens;

typedef enum {
	PARSER_OK = 0,
	PARSER_ERR_UNEXPECTED_TOKEN
} ParserError;

class Parser {
private:
	int index;
	const std::vector<Token> *tokens;

	Node* parse(Token token, int &error);
public:
	Parser(): index(0), tokens(nullptr) {}

	int parse(const Tokens& tokens, std::unique_ptr<Program>& program);
};

int parse(const Tokens& tokens, std::unique_ptr<Program>& program);

}

#endif
