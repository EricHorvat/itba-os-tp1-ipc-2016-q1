#ifndef __COMMUNICATION_CONNECTION_API_H__
#define __COMMUNICATION_CONNECTION_API_H__

#include <comm.errors.h>
#include <comm.addr.api.h>

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
	comm_addr_t*       client_addr;
	comm_addr_t*       server_addr;
	char*              connection_file;
	connection_state_t state;

	int req_fd;
	int res_fd;

	comm_sense_t sense;
} connection_t;

comm_error_code_t connection_open(connection_t* conn);
comm_error_code_t connection_close(connection_t* conn);

#define CONNECTION_OK 0

#define ERR_NULL_CONNECTION 200
#define ERR_CORRUPT_DATA 201
#define ERR_INVALID_ADDRESS 202
#define ERR_SERVER_OFFLINE 203
#define ERR_SERVER_BUSY 204
#define ERR_ 206

#define ERR_FIFO_FAIL_CONNECTION 300
#define ERR_FIFO_CANT_OPEN_RESPONSE 301
#define ERR_FIFO_CANT_OPEN_REQUEST 302

#define ERR_SOCKET_NOT_CREATED 400
#define ERR_NO_SUCH_HOST 401
#define ERR_SOCKET_NOT_CONNECTED 402

#endif