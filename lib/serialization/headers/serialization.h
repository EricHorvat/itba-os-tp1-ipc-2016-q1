#ifndef __SERIALIZATION_H__
#define __SERIALIZATION_H__

#include <communication.h>
#include <types.h>
#include <stdlib.h>

typedef struct {
	
	char *kind;

	/**
	 * uso esta horripilancia porque no se
	 * con que me puedo encontrar
	 */
	union {
		int i;
		double d;
		char *str;
		void *data;
		command_get_t *get_cmd;
		command_post_t *post_cmd;
		command_login_t *login_cmd;
		command_logout_t *logout_cmd;
		command_close_t *close_cmd;
	} data;

} parse_result_t;


// raw data
void send_string(char *string, connection_t *conn, comm_error_t *error);
void send_int(int number, connection_t *conn, comm_error_t *error);
void send_double(double number, connection_t *conn, comm_error_t *error);
void send_data(void *data, size_t size, connection_t *conn, comm_error_t *error);

// raw data async
void send_string_async(char *string, connection_t *conn, comm_callback_t cb);
void send_int_async(int number, connection_t *conn, comm_callback_t cb);
void send_double_async(double number, connection_t *conn, comm_callback_t cb);
void send_data_async(void *data, size_t size, connection_t *conn, comm_callback_t cb);

// commands
void send_cmd_get(command_get_t *cmd, connection_t *conn, comm_error_t *error);
void send_cmd_post(command_post_t *cmd, connection_t *conn, comm_error_t *error);
void send_cmd_login(command_login_t *cmd, connection_t *conn, comm_error_t *error);
void send_cmd_logout(command_logout_t *cmd, connection_t *conn, comm_error_t *error);
void send_cmd_close(command_close_t *cmd, connection_t *conn, comm_error_t *error);

// commands async
void send_cmd_get_async(command_get_t *cmd, connection_t *conn, comm_callback_t cb);
void send_cmd_post_async(command_post_t *cmd, connection_t *conn, comm_callback_t cb);

// stringify
const char* stringify_int(int);
const char* stringify_double(double);
const char* stringify_string(char*);
const char* stringify_data(void*, size_t);

const char* stringify_command_get(command_get_t *cmd);
const char* stringify_command_post(command_post_t *cmd);
const char* stringify_command_login(command_login_t *cmd);
const char* stringify_command_logout(command_logout_t *cmd);
const char* stringify_command_close(command_close_t *cmd);

// parse
parse_result_t *parse_encoded(const char *json);

// receive
parse_result_t *receive(connection_t *conn, comm_error_t *error);

#endif