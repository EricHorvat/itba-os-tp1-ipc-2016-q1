#ifndef __COMMUNICATION_ADDRESS_SOCKET_H__
#define __COMMUNICATION_ADDRESS_SOCKET_H__

#include <comm.addr.api.h>

typedef enum {
	ADDRESS_OK = 0,
	ADDRESS_INVALID_PROTOCOL,
	ADDRESS_MISSING_PROTOCOL_TOKEN,
	ADDRESS_NULL
} comm_addr_error_t;

typedef struct {
} comm_addr_extra_t;

#endif
