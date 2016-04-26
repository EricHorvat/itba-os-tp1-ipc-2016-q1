#ifndef __COMMUNICATION_CONNECTION_API__
#define __COMMUNICATION_CONNECTION_API__

#include <comm.errors.h>

typedef enum {
	CONNECTION_STATE_IDLE,
	CONNECTION_STATE_OPEN,
	CONNECTION_STATE_CLOSED
} connection_state_t;

typedef enum {
	COMMUNICATION_SERVER_CLIENT,
	COMMUNICATION_CLIENT_SERVER
} comm_sense_t;

typedef struct {
	comm_addr_t *client_addr;
	comm_addr_t *server_addr;
	char *connection_file;
	connection_state_t state;
} connection_t;

comm_error_code_t comm_open(connection_t *conn);
comm_error_code_t comm_close(connection_t *conn);

#endif