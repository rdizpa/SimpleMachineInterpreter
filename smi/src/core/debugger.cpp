#include "debugger.h"

#include "lexer.h"
#include "parser.h"

using smi::lexer::Tokens;

namespace smi::debugger {

int Debugger::load(const std::string& code) {
    Tokens tokens;

    if (tokenize(code, tokens) != lexer::LEXER_OK) {
        return DEBUGGER_ERR_INVALID_TOKEN;
    }

    if (parser::parse(tokens, program) != parser::PARSER_OK) {
        return DEBUGGER_ERR_UNEXPECTED_TOKEN;
    }

    this->pc = 0;

    for (Node* node : program->getBody()) {
        if (node->typeMatch(NodeType::LABEL)) {
            evalLabel(static_cast<Label*>(node));
        } else if (node->typeMatch(NodeType::ASSIGN)) {
            evalAssignment(static_cast<Assignment*>(node));
        }
        this->pc++;
    }

    this->pc = 0;
    this->advance();

    return DEBUGGER_OK;
}

void Debugger::advance() {
    while (this->pc < program->getBody().size() && program->getBody()[this->pc]->type() != NodeType::INSTR) {
        this->pc++;
    }
}

int Debugger::next() {
    if (!hasNext()) return DEBUGGER_ERR_EOF;

    Node* node = program->getBody()[this->pc];

    int ret = eval(node);

    if (ret == interpreter::INTERPRETER_ERR_UNDEFINED_LABEL) {
        return DEBUGGER_ERR_UNDEFINED_LABEL;
    } else if (ret != interpreter::INTERPRETER_OK) {
        return DEBUGGER_ERR_UNKNOWN;
    }

    this->pc++;
    this->advance();

    return DEBUGGER_OK;
}

int Debugger::getNextIndex() const {
    if (!hasNext()) return -1;

    return program->getBody()[this->pc]->index();
}

int Debugger::getNextLine() const {
    if (!hasNext()) return -1;

    return program->getBody()[this->pc]->line();
}

bool Debugger::hasNext() const {
    return this->pc < program->getBody().size();
}

}  // namespace smi::debugger
