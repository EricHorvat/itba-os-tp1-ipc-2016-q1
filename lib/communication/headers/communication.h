#ifndef __COMMUNICATION_H_
#define __COMMUNICATION_H_

#include "comm.addr.h"

typedef enum {
	CONNECTION_STATE_IDLE,
	CONNECTION_STATE_OPEN,
	CONNECTION_STATE_CLOSED
} connection_state_t;

typedef struct {
	comm_addr_t *client_addr;
	comm_addr_t *server_addr;
	char *connection_file;
	connection_state_t status;
} connection_t;


comm_error_code_t comm_open(connection_t *comm);
#include "comm.send.api.h"
#include "comm.receive.api.h"
comm_error_code_t comm_close(connection_t *address);

#endif