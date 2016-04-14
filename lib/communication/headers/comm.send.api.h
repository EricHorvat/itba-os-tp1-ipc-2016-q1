#ifndef __COMMUNICATION_SEND_API__
#define __COMMUNICATION_SEND_API__

#include "comm.addr.h"

typedef void (*comm_callback_t)(comm_error_t *err, comm_addr_t *origin, char *response);

/**
 * 
 *  SEND API
 * 
 * 
 **/

void send_message(char *message, comm_addr_t *addr, comm_error_t *error);
void send_data(void *data, unsigned int size, comm_addr_t *addr, comm_error_t *error);


void send_message_async(char *message, comm_addr_t *addr, comm_callback_t cb);
void send_data_async(void * data, unsigned int size, comm_addr_t *addr, comm_callback_t cb);

#endif

