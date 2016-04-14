#ifndef __SERIALIZARTION_H__
#define __SERIALIZARTION_H__

#include "../../communication/headers/communication.h"

void send_string(char *string, comm_addr_t *addr, comm_error_t *error);
void send_int(int number, comm_addr_t *addr, comm_error_t *error);
void send_float(float number, comm_addr_t *addr, comm_error_t *error);
void send_double(double number, comm_addr_t *addr, comm_error_t *error);
void send_char(char character, comm_addr_t *addr, comm_error_t *error);


void send_string_async(char *string, comm_addr_t *addr, comm_callback_t cb);
void send_int_async(int number, comm_addr_t *addr, comm_callback_t cb);
void send_float_async(float number, comm_addr_t *addr, comm_callback_t cb);
void send_double_async(double number, comm_addr_t *addr, comm_callback_t cb);
void send_char_async(char character, comm_addr_t *addr, comm_callback_t cb);



#endif