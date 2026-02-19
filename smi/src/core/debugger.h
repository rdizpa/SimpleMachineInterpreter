#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#include <memory>

#include "interpreter.h"

namespace smi::debugger {

typedef enum {
    DEBUGGER_OK,
    DEBUGGER_ERR_INVALID_TOKEN,
    DEBUGGER_ERR_UNEXPECTED_TOKEN,
    DEBUGGER_ERR_UNDEFINED_LABEL,
    DEBUGGER_ERR_EOF,
    DEBUGGER_ERR_UNKNOWN
} DebuggerError;

class Debugger : public smi::interpreter::Interpreter {
   private:
    std::unique_ptr<Program> program;

    void advance();

   public:
    int load(const std::string& code);
    int next();
    int getNextIndex() const;
    int getNextLine() const;
    bool hasNext() const;
};

}  // namespace smi::debugger

#endif
