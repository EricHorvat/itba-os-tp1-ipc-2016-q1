#ifndef __COMMUNICATION_RECEIVE_API_H__
#define __COMMUNICATION_RECEIVE_API_H__

#include <comm.addr.api.h>
#include <stdlib.h>
#include <comm.connection.api.h>

/**
 * 
 *  RECEIVE API
 * 
 * 
 **/

void comm_listen(connection_t* conn, comm_error_t* error);
void comm_accept(connection_t* conn, comm_error_t* error);
char* comm_receive_data(connection_t* conn, comm_error_t* error);
// comm_addr_t *server, comm_addr_t *client
#endif
