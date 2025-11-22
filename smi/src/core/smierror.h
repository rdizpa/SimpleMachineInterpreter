#ifndef _SMIERROR_H_
#define _SMIERROR_H_

#include <string>

namespace smi::error {

typedef struct {
    int index;
    int line;
    int column;
    int length;
    std::string message;
} ErrorData;

void setLastError(int index, int line, int column, int length, const std::string& message);
const smi::error::ErrorData& getLastError();
const std::string getLastErrorMessage();

}  // namespace smi::error

#endif
