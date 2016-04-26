#ifndef __SERIALIZARTION_H__
#define __SERIALIZARTION_H__

#include <communication.h>

#include <stdlib.h>

void send_string(char *string, connection_t *conn, comm_sense_t sense, comm_error_t *error);
void send_int(int number, connection_t *conn, comm_sense_t sense, comm_error_t *error);
void send_double(double number, connection_t *conn, comm_sense_t sense, comm_error_t *error);
void send_data(void *data, size_t size, connection_t *conn, comm_sense_t sense, comm_error_t *error);

void send_string_async(char *string, connection_t *conn, comm_sense_t sense, comm_callback_t cb);
void send_int_async(int number, connection_t *conn, comm_sense_t sense, comm_callback_t cb);
void send_double_async(double number, connection_t *conn, comm_sense_t sense, comm_callback_t cb);
void send_data_async(void *data, size_t size, connection_t *conn, comm_sense_t sense, comm_callback_t cb);



#endif