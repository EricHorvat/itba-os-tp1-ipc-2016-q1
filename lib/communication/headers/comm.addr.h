#ifndef __COMMUNICATION_ADDRESS_API__
#define __COMMUNICATION_ADDRESS_API__

#include "comm.errors.h"

typedef enum {
	ADDRESS_OK = 0,
	ADDRESS_MISSING_PROTOCOL_TOKEN,
	ADDRESS_MISSING_PORT_TOKEN,
	ADDRESS_INVALID_PORT
} comm_addr_error_t;

typedef struct {
	char *url;
	char *protocol;
	char *host;
	unsigned int port;
	unsigned char valid;
} comm_addr_t;

comm_addr_error_t address_from_url(char *url, comm_addr_t *address);


#endif

