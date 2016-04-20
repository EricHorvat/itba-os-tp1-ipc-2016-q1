#ifndef __SERIALIZARTION_H__
#define __SERIALIZARTION_H__

#include <communication.h>

#include <stdlib.h>

void send_string(char *string, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error);
void send_int(int number, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error);
void send_double(double number, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error);
void send_data(void *data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error);

void send_string_async(char *string, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb);
void send_int_async(int number, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb);
void send_double_async(double number, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb);
void send_data_async(void *data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb);



#endif