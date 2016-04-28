#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <types.h>

#include <serialization.h>
#include <utils.h>
#include <client_config.h>
#include <commands.h>

#define DEFAULT_PROTOCOL "fd"

static void response_handler(comm_error_t *err, connection_t *conn, char * response);
static int getrnduser();

static void response_handler(comm_error_t *err, connection_t *conn, char * response) {

	// printf("%s dice: %s\n", addr->url, response);

	parse_result_t *result;

	printf("response: (%s)\n", response);

	if (err->code) {
		printf(ANSI_COLOR_RED"server error(%d): %s\n"ANSI_COLOR_RESET, err->code, err->msg);
		return;
	}

	result = parse_encoded((const char*)response);

	if (strcmp(result->kind, "int") == 0) {
		printf(ANSI_COLOR_GREEN"server says: %d\n"ANSI_COLOR_RESET, result->data.i);
	} else if ( strcmp(result->kind, "double") == 0) {
		printf(ANSI_COLOR_GREEN"server says: %f\n"ANSI_COLOR_RESET, result->data.d);
	} else if ( strcmp(result->kind, "string") == 0 ) {
		printf(ANSI_COLOR_GREEN"server says: %s\n"ANSI_COLOR_RESET, result->data.str);
	}

	connection_close(conn);

}

static bool seeded = false;

static int getrnduser() {
	if (!seeded) {
		srand(time(NULL));
		seeded = true;
	}
	return rand() % 9000 + 1000;
}

int main (int argc, char **argv) {

/*
	nota para mi: el archivo para el server se va a llamar <servername>.in.fifo
	va a tener una lista de los fifo que tiene que dar bola
	el nombre va a ser <username>.req.fifo
	va a escribir su respuesta en <username>.res.fifo
*/
	connection_t *connection;

	comm_addr_error_t addr_error;
	char *client_url, *server_url;

	char *shell_buffer;
	int shell_index = 0;
	char c;
	bool command_read = false;

	command_get_t *get_cmd;

	int conn_error;

	int a;

	int err;

	int client_url_len;

	client_args_t *client_args;

	client_args = NEW(client_args_t);

	process_arguments(argc, argv, client_args);

	connection = NEW(connection_t);
	connection->client_addr = NEW(comm_addr_t);

	if (!client_args->client_name) {
		client_args->client_name = (char*)malloc(9);
		client_url_len = sprintf(client_args->client_name, "anon%d", ( a = getrnduser()));
		client_args->client_name[client_url_len] = 0;
	}
	if (!client_args->protocol) {
		client_args->protocol = DEFAULT_PROTOCOL;
	}

	// alocate url
	client_url = (char*)malloc(strlen(client_args->protocol)+3+strlen(client_args->client_name)+1);
	// format url
	client_url_len = sprintf(client_url, "%s://%s", client_args->protocol, client_args->client_name);
	client_url[client_url_len] = ZERO;

	printf("debug: {%s} {%s}\n", client_url, client_args->client_name);

	// build address
	if (address_from_url(client_url, connection->client_addr)) {
		printf(ANSI_COLOR_RED"address failed\n"ANSI_COLOR_RESET);
	}

	if (client_args->server) {
		connection->server_addr = NEW(comm_addr_t);
		server_url = (char*)malloc(strlen(client_args->protocol)+3+strlen(client_args->server));
		client_url_len = sprintf(server_url, "%s://%s", client_args->protocol, client_args->server);
		server_url[client_url_len] = 0;
	}

	// <-- log
	printf("+--------------------------------------------------------+\n");
	printf("| client name\t\t | server\t | protocol\t |\n");
	printf("+--------------------------------------------------------+\n");
	printf("| %s\t\t | %s\t | %s\t\t |\n", connection->client_addr->host, client_args->server, connection->client_addr->protocol);
	printf("+--------------------------------------------------------+\n");
	// <!-- end log

	shell_buffer = (char*)malloc(2048);
	memset(shell_buffer, ZERO, 2048);
/*
	while (1) {

		printf("> ");

		do {

			c = getchar();

			switch (c) {
				case NEWLINE:
					// putchar('\n');
					command_read = yes;
					break;
				case '\b':
					if (shell_index > 0)
						shell_index--;
					// putchar('\b');
					break;
				case '\r':
					break;
				case '\t':
					break;
				default:
					shell_buffer[shell_index++] = c;
					shell_buffer[shell_index] = 0;
					// putchar(c);
					break;
			}
		} while ( c != EOF && !command_read);

		if ( ( err = cmd_parse(connection, shell_buffer)) ) {
			printf(ANSI_COLOR_RED"cmd failed err: [%d]\n"ANSI_COLOR_RESET, err);
		}
		shell_buffer[0] = 0;
		command_read = no;
		shell_index = 0;

	}*/

	// connection->connection_file = "server_incoming_connections.fifo";

	
	server_url = "fd://google";
	connection->server_addr = NEW(comm_addr_t);
	address_from_url(server_url, connection->server_addr);

	// if ( (addr_error = address_from_url(, client_addr)) > 0) {

	// 	printf("Error %d\n", addr_error);
	// 	return addr_error;
	// }

	if ( (conn_error = connection_open(connection) != 0) ) {
		fprintf(stderr, ANSI_COLOR_RED"error %d creating connection\n"ANSI_COLOR_RESET, conn_error);
	}

	get_cmd = NEW(command_get_t);

	get_cmd->path = "/martin/home/etc";

	// send_cmd_get(get_cmd, connection, COMMUNICATION_CLIENT_SERVER, nil);

	send_int_async(1, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(2, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(3, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(4, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(5, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(6, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(7, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(8, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(9, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);
	send_int_async(0, connection, COMMUNICATION_CLIENT_SERVER, &response_handler);


	sleep(100);

}