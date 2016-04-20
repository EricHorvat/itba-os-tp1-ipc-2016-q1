#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

// #include "../lib/communication/headers/communication.h"
#include <serialization.h>
#include <utils.h>

#define DEFAULT_PROTOCOL "fd://"

typedef struct {
	char *client_name;
	char *server;
	char *protocol;
} client_args_t;

static void response_handler(comm_error_t *err, comm_addr_t *addr, char * response);
static void process_arguments(int argc, char **argv, client_args_t *);
static int getrnduser();

static void response_handler(comm_error_t *err, comm_addr_t *addr, char * response) {

	// printf("%s dice: %s\n", addr->url, response);

	printf("Err Code: %d\tmsg: %s\n", err->code, err->msg);
	printf("response: %s\n", response);

}

static void process_arguments(int argc, char **argv, client_args_t *client_args) {

	// http://www.gnu.org/software/libc/manual/html_falsede/Example-of-Getopt.html#Example-of-Getopt

	int index;
	int c;

	if (!client_args)
		client_args = NEW(client_args_t);

	client_args->client_name = NULL;
	client_args->server = NULL;
	client_args->protocol = NULL;

	opterr = 0;
	while ((c = getopt(argc, argv, "n:s:p:")) != -1) {
		switch (c) {
			case 'n':
				client_args->client_name = optarg;
				break;
			case 's':
				client_args->server = optarg;
				break;
			case 'p':
				
				if ( strcmp(optarg, "fd") != 0 && strcmp(optarg, "socket") != 0) {
					fprintf(stderr, "%s is not an allowed protocol\n", optarg);
					abort();
				}

				client_args->protocol = optarg;
				break;
			case '?':
				if (optopt == 'n')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				if (optopt == 's')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				if (optopt == 'p')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt))
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				return;
			default:
				printf("Aborting\n");
				abort();
		}
	}

	for (index = optind; index < argc; index++)
		printf ("non-option argument %s\n", argv[index]);

	return;
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
	// connection_t *connection;

	comm_addr_t *client_addr, *server_addr;
	comm_addr_error_t addr_error;

	client_args_t *client_args;

	client_args = NEW(client_args_t);

	process_arguments(argc, argv, client_args);

	printf("Argument info:\nClient name: %s\tServer: %s\tProtocol:%s\n", client_args->client_name, client_args->server, client_args->protocol);

	if (!client_args->client_name) {
		client_args->client_name = (char*)malloc(9);
		sprintf(client_args->client_name, "anon%d", getrnduser());
	}

	if (!client_args->protocol) {
		client_args->protocol = DEFAULT_PROTOCOL;
	}

	if (!client_args->server) {
		// entrar en modo interactivo
		client_args->server = "google.com";
	}

	printf("Argument info:\nClient name: %s\tServer: %s\tProtocol:%s\n", client_args->client_name, client_args->server, client_args->protocol);

	// connection = NEW(connection_t);
	client_addr = NEW(comm_addr_t);
	server_addr = NEW(comm_addr_t);

	// connection->addr = addr;
	// connection->connection_file = "server_incoming_connections.fifo";

	if ( (addr_error = address_from_url(sprintf(), client_addr)) > 0) {

		printf("Error %d\n", addr_error);
		return addr_error;
	}

	comm_open(connection);

	printf("Address Info:\nProtocol: %s\tHost: %s\tPort:%d\n", addr->protocol, addr->host, addr->port);

	send_int_async(3, client_addr, server_addr, &response_handler);

	printf("Something\n");

}