#ifndef __COMMUNICATION_RECEIVE_API__
#define __COMMUNICATION_RECEIVE_API__

#include <comm.addr.h>
#include <stdlib.h>

/**
 * 
 *  RECEIVE API
 * 
 * 
 **/

connection_t* comm_listen(comm_addr_t *server, comm_error_t *error);
char* comm_receive_data(comm_addr_t *server, comm_addr_t *client, comm_error_t *error);

#endif

