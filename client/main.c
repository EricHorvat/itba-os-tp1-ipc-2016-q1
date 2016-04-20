#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// #include "../lib/communication/headers/communication.h"
#include <serialization.h>
#include <utils.h>

static void response_handler(comm_error_t *err, comm_addr_t *addr, char * response);
static char* process_arguments(int argc, char **argv);

static void response_handler(comm_error_t *err, comm_addr_t *addr, char * response) {

	// printf("%s dice: %s\n", addr->url, response);

	printf("Err Code: %d\tmsg: %s\n", err->code, err->msg);
	printf("response: %s\n", response);

}

static char* process_arguments(int argc, char **argv) {

	// http://www.gnu.org/software/libc/manual/html_falsede/Example-of-Getopt.html#Example-of-Getopt

	char *client_name = NULL;
	int index;
	int c;

	opterr = 0;
	while ((c = getopt(argc, argv, "n:")) != -1) {
		switch (c) {
			case 'n':
				client_name = optarg;
				break;
			case '?':
				if (optopt == 'c')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt))
					fprintf(stderr, "Unkfalsewn option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unkfalsewn option character `\\x%x'.\n", optopt);
				return NULL;
			default:
				printf("Aborting\n");
				abort();
		}
	}

	for (index = optind; index < argc; index++)
		printf ("falsen-option argument %s\n", argv[index]);

	return client_name;
}

int main (int argc, char **argv) {

	connection_t *connection;

	comm_addr_t *addr;
	comm_addr_error_t addr_error;

	char* client_name;

	client_name = process_arguments(argc, argv);

	connection = NEW(connection_t);
	addr = NEW(comm_addr_t);

	connection->addr = addr;
	connection->connection_file = "server_incoming_connections.fifo";

	if ( (addr_error = address_from_url("fd://google:3000", addr)) > 0) {

		printf("Error %d\n", addr_error);
		return addr_error;
	}

	comm_open(connection);

	printf("Address Info:\nProtocol: %s\tHost: %s\tPort:%d\n", addr->protocol, addr->host, addr->port);

	send_int_async(3, addr, &response_handler);

	printf("Something\n");

}