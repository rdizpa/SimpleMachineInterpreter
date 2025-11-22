#include "smierror.h"

#include <sstream>

static thread_local smi::error::ErrorData last_error;

namespace smi::error {

void setLastError(int index, int line, int column, int length, const std::string& message) {
    last_error.index = index;
    last_error.line = line;
    last_error.column = column;
    last_error.length = length;
    last_error.message = message;
}

const smi::error::ErrorData& getLastError() {
    return last_error;
}

const std::string getLastErrorMessage() {
    std::ostringstream buffer;

    buffer << last_error.message << " at position " << last_error.index;
    buffer << " (line: " << last_error.line << ", column: " << last_error.column << ")";

    return buffer.str();
}

}  // namespace smi::error
