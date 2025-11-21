#include "interpreter.h"

#include "lexer.h"
#include "parser.h"

namespace smi::interpreter {

using lexer::Tokens;

std::vector<std::string> Interpreter::getMemoryKeys() {
    std::vector<std::string> keys;

    for (auto pair : this->memory) {
        keys.push_back(pair.first);
    }

    return keys;
}

uint16_t Interpreter::getMemoryValue(std::string key) {
    return this->memory.at(key);
}

int Interpreter::eval(const std::string& code) {
    Tokens tokens;

    if (tokenize(code, tokens) != lexer::LEXER_OK) {
        return INTERPRETER_ERR_INVALID_TOKEN;
    }

    std::unique_ptr<Program> program;

    if (parser::parse(tokens, program) != parser::PARSER_OK) {
        return INTERPRETER_ERR_UNEXPECTED_TOKEN;
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

    while (this->pc < program->getBody().size()) {
        Node* node = program->getBody()[this->pc];

        int ret = eval(node);

        if (ret != 0) {
            return ret;
        }

        this->pc++;
    }

    return INTERPRETER_OK;
}

int Interpreter::eval(Node* node) {
    if (node->type() == NodeType::INSTR) {
        return evalInstruction(static_cast<Instruction*>(node));
    }

    return INTERPRETER_OK;
}

void Interpreter::evalLabel(Label* ls) {
    labels[ls->getName()] = this->pc;
}

int Interpreter::evalInstruction(Instruction* inst) {
    // std::string op1 = static_cast<Identifier*>(inst->getOp1())->getValue();
    // std::string op2;
    const Identifier* op1 = static_cast<Identifier*>(inst->getOp1());
    const Identifier* op2 = nullptr;
    std::string op1_val = op1->getValue();
    std::string op2_val;

    if (inst->getInstr() == "MOV") {
        op2 = static_cast<Identifier*>(inst->getOp2());
        op2_val = op2->getValue();

        if (memory.find(op1_val) == memory.end() || memory.find(op2_val) == memory.end()) {
            const Identifier* opNotFound = (memory.find(op1_val) != memory.end()) ? op2 : op1;

            THROW_LABEL_NOT_FOUND_EXC(
                opNotFound->getValue(), opNotFound->index(), opNotFound->line(), opNotFound->column()

            );

            return INTERPRETER_ERR_UNDEFINED_LABEL;
        }

        memory[op2_val] = memory[op1_val];
    } else if (inst->getInstr() == "ADD") {
        op2 = static_cast<Identifier*>(inst->getOp2());
        op2_val = op2->getValue();

        if (memory.find(op1_val) == memory.end() || memory.find(op2_val) == memory.end()) {
            const Identifier* opNotFound = (memory.find(op1_val) != memory.end()) ? op2 : op1;

            THROW_LABEL_NOT_FOUND_EXC(
                opNotFound->getValue(), opNotFound->index(), opNotFound->line(), opNotFound->column()

            );

            return INTERPRETER_ERR_UNDEFINED_LABEL;
        }

        memory[op2_val] += memory[op1_val];
    } else if (inst->getInstr() == "CMP") {
        op2 = static_cast<Identifier*>(inst->getOp2());
        op2_val = op2->getValue();

        if (memory.find(op1_val) == memory.end() || memory.find(op2_val) == memory.end()) {
            const Identifier* opNotFound = (memory.find(op1_val) != memory.end()) ? op2 : op1;

            THROW_LABEL_NOT_FOUND_EXC(
                opNotFound->getValue(), opNotFound->index(), opNotFound->line(), opNotFound->column()

            );

            return INTERPRETER_ERR_UNDEFINED_LABEL;
        }

        this->cmp = memory[op1_val] == memory[op2_val];
    } else if (inst->getInstr() == "BEQ") {
        if (labels.find(op1_val) == labels.end()) {
            THROW_LABEL_NOT_FOUND_EXC(op1_val, op1->index(), op1->line(), op1->column());
            return INTERPRETER_ERR_UNDEFINED_LABEL;
        }

        if (this->cmp) {
            this->pc = labels[op1_val];
        }
    }

    return INTERPRETER_OK;
}

void Interpreter::evalAssignment(Assignment* as) {
    memory[as->getName()] = std::stoi(as->getValue(), 0, 16);
}

}  // namespace smi::interpreter
