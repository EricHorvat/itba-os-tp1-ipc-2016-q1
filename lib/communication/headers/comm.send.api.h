#ifndef __COMMUNICATION_SEND_API__
#define __COMMUNICATION_SEND_API__

#include "comm.addr.h"

/**
 * 
 *  SEND API
 * 
 * 
 **/

void send_message(char *message, comm_addr_t *addr, comm_error_t *error);
void send_int(int number, comm_addr_t *addr, comm_error_t *error);
void send_double(int number, comm_addr_t *addr, comm_error_t *error);
void send_char(char character, comm_addr_t *addr, comm_error_t *error);
void send_data(void *data, unsigned int size, comm_addr_t *addr, comm_error_t *error);


void send_message_async(char *message, comm_addr_t *addr, comm_callback_t cb);
void send_int_async(int number, comm_addr_t *addr, comm_callback_t cb);
void send_double_async(int number, comm_addr_t *addr, comm_callback_t cb);
void send_char_async(char character, comm_addr_t *addr, comm_callback_t cb);
void send_data_async(void * data, unsigned int size, comm_addr_t *addr, comm_callback_t cb);

#endif

