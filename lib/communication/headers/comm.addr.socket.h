#ifndef __COMMUNICATION_ADDRESS_SOCKET_H__
#define __COMMUNICATION_ADDRESS_SOCKET_H__

#include <comm.addr.api.h>

typedef enum {
	ADDRESS_OK = 0,
	ADDRESS_INVALID_PROTOCOL,
	ADDRESS_MISSING_PROTOCOL_TOKEN,
	ADDRESS_MISSING_PORT_TOKEN,
	ADDRESS_INVALID_PORT,
	ADDRESS_NULL
} comm_addr_error_t;

typedef struct {
	unsigned int port;
	// sock_addr_in
} comm_addr_extra_t;

#endif
