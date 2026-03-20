#include "interpreter.h"

#include <algorithm>

#include "lexer.h"
#include "parser.h"

namespace smi::interpreter {

using lexer::Tokens;

const std::vector<std::string>& Interpreter::getMemoryKeys() {
    return this->memory_keys;
}

uint16_t Interpreter::getMemoryValue(std::string key) {
    return this->memory.at(key);
}

int Interpreter::eval(const std::string& code) {
    Tokens tokens;

    if (tokenize(code, tokens) != lexer::LEXER_OK) {
        return INTERPRETER_ERR_INVALID_TOKEN;
    }

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

        if (isMemoryLabel(op1_val) && isMemoryLabel(op2_val)) {
            memory[op2_val] = memory[op1_val];
        } else if (isCodeLabel(op1_val) && isCodeLabel(op2_val)) {
            Node* newNode = program->getBody()[labels[op1_val] + 1]->clone();

            delete program->getBody()[labels[op2_val] + 1];

            program->getBody()[labels[op2_val] + 1] = newNode;
        } else if (isMemoryLabel(op1_val) && isCodeLabel(op2_val) || isCodeLabel(op1_val) && isMemoryLabel(op2_val)) {
            THROW_INCOMPATIBLE_LABEL_TYPE_EXC(op2->getValue(), op2->index(), op2->line(), op2->column());

            return INTERPRETER_ERR_INCOMPATIBLE_LABEL_TYPE;
        } else {
            const Identifier* opNotFound = (isMemoryLabel(op1_val) || isCodeLabel(op1_val)) ? op2 : op1;

            THROW_LABEL_NOT_FOUND_EXC(opNotFound->getValue(), opNotFound->index(), opNotFound->line(),
                                      opNotFound->column());

            return INTERPRETER_ERR_UNDEFINED_LABEL;
        }
    } else if (inst->getInstr() == "ADD") {
        op2 = static_cast<Identifier*>(inst->getOp2());
        op2_val = op2->getValue();

        if (!isMemoryLabel(op1_val) || !isMemoryLabel(op2_val)) {
            const Identifier* opNotFound = isMemoryLabel(op1_val) ? op2 : op1;

            if (isCodeLabel(op1_val) || isCodeLabel(op2_val)) {
                THROW_INCOMPATIBLE_LABEL_TYPE_EXC(opNotFound->getValue(), opNotFound->index(), opNotFound->line(),
                                                  opNotFound->column());
                return INTERPRETER_ERR_INCOMPATIBLE_LABEL_TYPE;
            }

            THROW_LABEL_NOT_FOUND_EXC(opNotFound->getValue(), opNotFound->index(), opNotFound->line(),
                                      opNotFound->column());

            return INTERPRETER_ERR_UNDEFINED_LABEL;
        }

        memory[op2_val] += memory[op1_val];
    } else if (inst->getInstr() == "CMP") {
        op2 = static_cast<Identifier*>(inst->getOp2());
        op2_val = op2->getValue();

        if (isMemoryLabel(op1_val) && isMemoryLabel(op2_val)) {
            this->cmp = memory[op1_val] == memory[op2_val];
        } else if (isCodeLabel(op1_val) && isCodeLabel(op2_val)) {
            this->cmp = *program->getBody()[labels[op2_val] + 1] == *program->getBody()[labels[op1_val] + 1];
        } else if (isMemoryLabel(op1_val) && isCodeLabel(op2_val) || isCodeLabel(op1_val) && isMemoryLabel(op2_val)) {
            THROW_INCOMPATIBLE_LABEL_TYPE_EXC(op2->getValue(), op2->index(), op2->line(), op2->column());

            return INTERPRETER_ERR_INCOMPATIBLE_LABEL_TYPE;
        } else {
            const Identifier* opNotFound = (isMemoryLabel(op1_val) || isCodeLabel(op1_val)) ? op2 : op1;

            THROW_LABEL_NOT_FOUND_EXC(opNotFound->getValue(), opNotFound->index(), opNotFound->line(),
                                      opNotFound->column());

            return INTERPRETER_ERR_UNDEFINED_LABEL;
        }
    } else if (inst->getInstr() == "BEQ") {
        if (!isCodeLabel(op1_val)) {
            if (isMemoryLabel(op1_val)) {
                THROW_INCOMPATIBLE_LABEL_TYPE_EXC(op1_val, op1->index(), op1->line(), op1->column());
                return INTERPRETER_ERR_INCOMPATIBLE_LABEL_TYPE;
            }

            THROW_LABEL_NOT_FOUND_EXC(op1_val, op1->index(), op1->line(), op1->column());
            return INTERPRETER_ERR_UNDEFINED_LABEL;
        }

        if (this->cmp) {
            this->pc = labels[op1_val];
        }
    }

    this->executedInstructions++;

    return INTERPRETER_OK;
}

void Interpreter::evalAssignment(Assignment* as) {
    memory[as->getName()] = std::stoi(as->getValue(), 0, 16);

    if (std::find(memory_keys.begin(), memory_keys.end(), as->getName()) == memory_keys.end())
        memory_keys.push_back(as->getName());
}

}  // namespace smi::interpreter
