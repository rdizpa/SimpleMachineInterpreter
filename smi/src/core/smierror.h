#ifndef _SMIERROR_H_
#define _SMIERROR_H_

#include <string>

void setLastError(const std::string& error);
const std::string& getLastError();

#endif
