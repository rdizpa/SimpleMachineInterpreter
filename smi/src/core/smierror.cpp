#include "smierror.h"

static thread_local std::string last_error;

void setLastError(const std::string& error) {
    last_error = error;
}

const std::string& getLastError() {
    return last_error;
}
