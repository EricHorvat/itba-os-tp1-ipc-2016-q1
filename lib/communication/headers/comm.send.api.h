#ifndef __COMMUNICATION_SEND_API__
#define __COMMUNICATION_SEND_API__

#include <comm.addr.h>
#include <comm.connection.api.h>
#include <stdlib.h>

typedef void (*comm_callback_t)(comm_error_t *err, comm_addr_t *origin, char *response);

/**
 * 
 *  SEND API
 * 
 * 
 **/

void comm_send_data(void *data, size_t size, connection_t *conn, comm_sense_t sense, comm_error_t *error);
void comm_send_data_async(void * data, size_t size, connection_t *conn, comm_sense_t sense, comm_callback_t cb);

#endif

