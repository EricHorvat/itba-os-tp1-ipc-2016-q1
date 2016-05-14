#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <types.h>
#include <serialization.h>
#include <utils.h>
#include <client_config.h>
#include <commands.h>
#include <file_utils.h>

#define DEFAULT_PROTOCOL "socket"

static void response_handler(comm_error_t* err, connection_t* conn, char* response);

static void response_handler(comm_error_t* err, connection_t* conn, char* response) {

	// printf("%s dice: %s\n", addr->url, response);

	parse_result_t* result;

	LOG("response: (%s)\n", response);

	if (err->code) {
		printf(ANSI_COLOR_RED "server error(%d): %s\n" ANSI_COLOR_RESET, err->code, err->msg);
		return;
	}

	result = parse_encoded((const char*)response);

	if (strcmp(result->kind, "int") == 0) {
		SUCCESS("server says: %d\n", result->data.i);
	} else if (strcmp(result->kind, "double") == 0) {
		SUCCESS("server says: %f\n", result->data.d);
	} else if (strcmp(result->kind, "string") == 0) {
		SUCCESS("server says: %s\n", result->data.str);
	}

	connection_close(conn);
}

int main(int argc, char** argv) {

	connection_t* connection;

	comm_addr_error_t addr_error;
	char *            client_url, *server_url;

	char* shell_buffer;
	int   shell_index = 0;
	char  c;
	bool  command_read = false;

	int rnd;
	int err;
	int client_url_len;

	client_args_t* client_args;

	client_args = NEW(client_args_t);

	process_arguments(argc, argv, client_args);

	connection              = NEW(connection_t);
	connection->client_addr = NEW(comm_addr_t);

	if (!client_args->client_name) {
		client_args->client_name = (char*)malloc(9);
		client_url_len = sprintf(client_args->client_name, "anon%d", (rnd = getrnd(1000, 10000)));
		client_args->client_name[client_url_len] = 0;
	}
	if (!client_args->protocol) {
		client_args->protocol = DEFAULT_PROTOCOL;
	}

	// alocate url
	client_url = (char*)malloc(strlen(client_args->protocol) + 3 + strlen(client_args->client_name) + 1 + 5);
	// format url
	client_url_len             = sprintf(client_url, "%s://%s:3000", client_args->protocol, client_args->client_name);
	client_url[client_url_len] = ZERO;

	// build address
	if ((addr_error = address_from_url(client_url, connection->client_addr))) {
		ERROR("Address failed");
	}

	if (client_args->server) {
		connection->server_addr    = NEW(comm_addr_t);
		server_url                 = (char*)malloc(strlen(client_args->protocol) + 3 + strlen(client_args->server));
		client_url_len             = sprintf(server_url, "%s://%s", client_args->protocol, client_args->server);
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
	/**/
	while (!isConnectionClosed(connection)) {

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
					shell_buffer[shell_index]   = 0;
					// putchar(c);
					break;
			}
		} while (c != EOF && !command_read);

		if ((err = cmd_parse(connection, shell_buffer))) {
			ERROR("Command failed with error: %d", err);
		}
		shell_buffer[0] = 0;
		command_read    = no;
		shell_index     = 0;
	}
	//close resources: DB, MSQ, FIFO
	
	return 0;
}