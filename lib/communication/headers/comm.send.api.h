#ifndef __COMMUNICATION_SEND_API__
#define __COMMUNICATION_SEND_API__

#include <comm.addr.h>
#include <stdlib.h>

typedef void (*comm_callback_t)(comm_error_t *err, comm_addr_t *origin, char *response);

/**
 * 
 *  SEND API
 * 
 * 
 **/

void set_timeout(unsigned int);

void comm_send_data(void *data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error);
void comm_send_data_async(void * data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb);

#endif

