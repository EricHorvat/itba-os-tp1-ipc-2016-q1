#ifndef __COMMUNICATION_SEND_API_H__
#define __COMMUNICATION_SEND_API_H__

#include <comm.addr.api.h>
#include <comm.connection.api.h>
#include <stdlib.h>

typedef void (*comm_callback_t)(comm_error_t *err, connection_t *conn, char *response);

/**
 * 
 *  SEND API
 * 
 * 
 **/

void comm_send_data(void *data, size_t size, connection_t *conn, comm_error_t *error);
void comm_send_data_async(void * data, size_t size, connection_t *conn, comm_callback_t cb);

#endif

