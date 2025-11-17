#include "parser.h"

#include "smierror.h"

#include <sstream>

namespace smi::parser {

using lexer::TokenType;

int parse(const Tokens& tokens, std::unique_ptr<Program>& program) {
	Parser parser;
	return parser.parse(tokens, program);
}

int Parser::parse(const Tokens& tokens, std::unique_ptr<Program>& program) {
	this->tokens = &tokens;
	index = 0;
	std::vector<Node*> body;
	int error = PARSER_OK;

	while (index < tokens.size()) {
		body.push_back(parse(tokens[index], error));
		index++;
	}

	program = std::make_unique<Program>(body, 0, 1, 1);

	return error;
}

Node* Parser::parse(Token token, int &error) {
	if (token.type == TokenType::LABEL) {
		if (index + 1 < tokens->size() && (*tokens)[index + 1].type == TokenType::LITERAL) {
			index++;
			return new Assignment(token.value, (*tokens)[index].value, token.index, token.line, token.column);
		}

		return new Label(token.value, token.index, token.line, token.column);
	}
	else if (token.type == TokenType::INSTR) {
		index++;

		Node* op1 = parse((*tokens)[index], error);

		if (token.value != "BEQ" && (*tokens)[++index].type != TokenType::COMMA) {
			std::ostringstream buffer;
			buffer << "Unexpected token '" << (*tokens)[index].value << "', expected ',' at position ";
			buffer << (*tokens)[index].index << " (line: " << (*tokens)[index].line;
			buffer << ", column: " << (*tokens)[index].column << ")";

			setLastError(buffer.str());

			error = PARSER_ERR_UNEXPECTED_TOKEN;
		}

		Node* op2 = (token.value == "BEQ") ? nullptr : parse((*tokens)[++index], error);

		return new Instruction(token.value, op1, op2, token.index, token.line, token.column);
	}
	else if (token.type == TokenType::OP) {
		return new Identifier(token.value, token.index, token.line, token.column);
	}
	
	return nullptr;
}

}
