#include "parser.h"

#include <sstream>

#include "smierror.h"

namespace smi::parser {

int parse(const Tokens& tokens, std::unique_ptr<Program>& program) {
    Parser parser;
    return parser.parse(tokens, program);
}

int Parser::parse(const Tokens& tokens, std::unique_ptr<Program>& program) {
    this->tokens = &tokens;
    index = 0;
    std::vector<Node*> body;
    int error = PARSER_OK;

    while (index < tokens.size() && tokens[index].type != TokenType::_EOF) {
        body.push_back(parse(tokens[index], error));

        if (error != PARSER_OK) {
            break;
        }

        index++;
    }

    program = std::make_unique<Program>(body, 0, 1, 1);

    return error;
}

Node* Parser::parse(Token token, int& error) {
    if (token.type == TokenType::LABEL) {
        return parseLabel(token, error);
    } else if (token.type == TokenType::INSTR) {
        return parseInstr(token, error);
    }

    std::ostringstream buffer;
    buffer << "Unexpected token '" << ((token.type != TokenType::_EOF) ? token.value : "EOF") << "'";

    smi::error::setLastError(token.index, token.line, token.column, token.value.length(), buffer.str());

    error = PARSER_ERR_UNEXPECTED_TOKEN;

    return nullptr;
}

Node* Parser::parseOperand(Token token, int& error) {
    return new Identifier(token.value, token.index, token.line, token.column);
}

Node* Parser::parseLabel(Token token, int& error) {
    int err;
    if (tokenMatch((*tokens)[index + 1], TokenType::LITERAL, err)) {
        index++;
        return new Assignment(token.value, (*tokens)[index].value, token.index, token.line, token.column);
    }

    return new Label(token.value, token.index, token.line, token.column);
}

Node* Parser::parseInstr(Token token, int& error) {
    index++;

    if (!tokenMatch((*tokens)[index], TokenType::OP, error)) return nullptr;

    Node* op1 = parseOperand((*tokens)[index], error);

    if (token.value != "BEQ" && !tokenMatch((*tokens)[++index], TokenType::COMMA, error)) {
        delete op1;
        return nullptr;
    }

    if (token.value != "BEQ" && !tokenMatch((*tokens)[index + 1], TokenType::OP, error)) {
        delete op1;
        return nullptr;
    }

    Node* op2 = (token.value == "BEQ") ? nullptr : parseOperand((*tokens)[++index], error);

    return new Instruction(token.value, op1, op2, token.index, token.line, token.column);
}

bool Parser::tokenMatch(const Token& token, TokenType type, int& error) {
    if (token.type != type) {
        std::string tk_exp;

        switch (type) {
            case TokenType::COMMA:
                tk_exp = "','";
                break;
            case TokenType::INSTR:
                tk_exp = "an instruction";
                break;
            case TokenType::LABEL:
                tk_exp = "a label";
                break;
            case TokenType::LITERAL:
                tk_exp = "a number";
                break;
            case TokenType::OP:
                tk_exp = "an operand";
                break;
            case TokenType::_EOF:
                tk_exp = "end of file";
                break;
            default:
                tk_exp = "";
        }

        std::ostringstream buffer;
        buffer << "Unexpected token '" << ((token.type != TokenType::_EOF) ? token.value : "EOF") << "', expected "
               << tk_exp;

        smi::error::setLastError(token.index, token.line, token.column, token.value.length(), buffer.str());

        error = PARSER_ERR_UNEXPECTED_TOKEN;
        return false;
    }

    return true;
}

}  // namespace smi::parser
