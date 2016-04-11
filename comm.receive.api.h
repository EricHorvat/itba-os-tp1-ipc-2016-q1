#ifndef __COMMUNICATION_RECEIVE_API__
#define __COMMUNICATION_RECEIVE_API__

/**
 * 
 *  RECEIVE API
 * 
 * 
 **/

void receive_message(char *message, comm_addr_t *addr, comm_error_t *error);
void receive_int(int number, comm_addr_t *addr, comm_error_t *error);
void receive_double(int number, comm_addr_t *addr, comm_error_t *error);
void receive_char(char character, comm_addr_t *addr, comm_error_t *error);
void receive_data(void* data, unsigned int size, comm_addr_t *addr, comm_error_t *error);


#endif

