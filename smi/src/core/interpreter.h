#ifndef _INTERPRETER_H_
#define _INTERPRETER_H_

#include <cstdint>
#include <sstream>
#include <string>
#include <unordered_map>

#include "ast.h"
#include "smierror.h"

namespace smi::interpreter {

typedef enum {
    INTERPRETER_OK = 0,
    INTERPRETER_ERR_UNDEFINED_LABEL,
    INTERPRETER_ERR_INVALID_TOKEN,
    INTERPRETER_ERR_UNEXPECTED_TOKEN
} InterpreterError;

class Interpreter {
   private:
    std::unordered_map<std::string, uint16_t> memory;
    std::unordered_map<std::string, uint16_t> labels;
    bool cmp;
    int pc;

    int eval(Node* node);
    void evalLabel(Label* ls);
    int evalInstruction(Instruction* inst);
    void evalAssignment(Assignment* as);

   public:
    int eval(const std::string& code);
    std::vector<std::string> getMemoryKeys();
    uint16_t getMemoryValue(std::string key);
};

}  // namespace smi::interpreter

#define THROW_LABEL_NOT_FOUND_EXC(label, pos, line, column) \
    std::ostringstream buffer;                              \
    buffer << "Undefined label '" << label;                 \
    buffer << "' at position " << pos;                      \
    buffer << " (line: " << line << ", column: ";           \
    buffer << column << ")";                                \
    setLastError(buffer.str());

#endif
