#ifndef _SMI_COMMON_ERRORS_
#define _SMI_COMMON_ERRORS_

typedef enum {
    SMI_OK = 0,
    SMI_ERR_UNDEFINED_LABEL,
    SMI_ERR_INVALID_TOKEN,
    SMI_ERR_UNEXPECTED_TOKEN,
    SMI_ERR_UNKNOWN
} SMIError;

typedef struct {
    int index;
    int line;
    int column;
    int length;
    const char* message;
} SMIErrorData;

#endif
