#ifndef __COMMUNICATION_ERRORS_API__
#define __COMMUNICATION_ERRORS_API__

typedef struct {
	int   code;
	char* msg;
} comm_error_t;

typedef enum {
	COMM_ERROR_UNKWOWN = -1,
	COMM_ERROR_NONE
} comm_error_code_t;

#endif
