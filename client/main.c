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

int main(int argc, char** argv) {

	connection_t* connection;

	comm_addr_error_t addr_error;
	char *            client_url;

	char* shell_buffer;
	int   shell_index = 0;
	char  c;
	boolean  command_read = false;

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
#ifdef __FIFO__
	client_args->protocol = "fifo";
#else
	client_args->protocol = "socket";
#endif

	// alocate url
	client_url = (char*)malloc(strlen(client_args->protocol) + 3 + strlen(client_args->client_name) + 1 + 5);
	// format url
	client_url_len             = sprintf(client_url, "%s://%s:3000", client_args->protocol, client_args->client_name);
	client_url[client_url_len] = ZERO;

	// build address
	if ((addr_error = address_from_url(client_url, connection->client_addr))) {
		ERROR("Address failed");
	}

	// <-- log
	SUCCESS("Welcome to:");
	ERROR("\n_____________________    _________________    .______________________ \n"
"\\__    ___/\\______   \\  /   _____/\\_____  \\   |   \\______   \\_   ___ \\ \n"
"  |    |    |     ___/  \\_____  \\  /   |   \\  |   ||     ___/    \\  \\/ \n"
"  |    |    |    |      /        \\/    |    \\ |   ||    |   \\     \\____\n"
"  |____|    |____|     /_______  /\\_______  / |___||____|    \\______  /\n"
"                               \\/         \\/                        \\/ \n"
"___________.__.__             _________                                \n"
"\\_   _____/|__|  |   ____    /   _____/ ______________  __ ___________ \n"
" |    __)  |  |  | _/ __ \\   \\_____  \\_/ __ \\_  __ \\  \\/ // __ \\_  __ \\\n"
" |     \\   |  |  |_\\  ___/   /        \\  ___/|  | \\/\\   /\\  ___/|  | \\/\n"
" \\___  /   |__|____/\\___  > /_______  /\\___  >__|    \\_/  \\___  >__|   \n"
"     \\/                 \\/          \\/     \\/                 \\/ \n");
#ifdef __FIFO__
	LOG("This is a FIFO based client");
	INFO("URl Schema is fifo://<server-name>");
#else
	LOG("This is a SOCKET based client");
	INFO("URl Schema is socket://<server-name>:<port>");
#endif
	
	INFO("You're username is %s:", client_args->client_name);

	INFO("Feel free to use the help command.");
	// <!-- end log

	shell_buffer = (char*)malloc(2048);
	memset(shell_buffer, ZERO, 2048);
	

	while (!is_connection_closed(connection)) {

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
			if (err != COMMAND_CLOSE_OK) {
				ERROR("Command failed with error: %d", err);
			} else {
				connection_close(connection);
			}
		}
		shell_buffer[0] = 0;
		command_read    = no;
		shell_index     = 0;
	}
	//close resources: DB, MSQ, FIFO

	return 0;
}
