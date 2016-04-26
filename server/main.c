#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

#include <communication.h>

#include <config/server_config.h>

#include <utils.h>

static char* process_arguments(int argc, char **argv);

static char* process_arguments(int argc, char **argv) {

	// http://www.gnu.org/software/libc/manual/html_falsede/Example-of-Getopt.html#Example-of-Getopt

	char *config_file_opt = NULL;
	int index;
	int c;

	opterr = 0;
	while ((c = getopt(argc, argv, "c:")) != -1) {
		switch (c) {
			case 'c':
				config_file_opt = optarg;
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

	return config_file_opt;
}

static void listen_connections(server_config_t *config) {

	connection_t *connection;
	comm_addr_t *server_addr, *client_addr;
	pid_t childPID;
	char *command;
	comm_error_t *err;

	err = NEW(comm_error_t);
	server_addr = NEW(comm_addr_t);
	client_addr = NEW(comm_addr_t);

	if (address_from_url("fd://google", server_addr) != 0) {
		fprintf(stderr, ANSI_COLOR_RED "Invalid Address\n" ANSI_COLOR_RESET);
		abort();
	}

	printf(ANSI_COLOR_GREEN"listening on name: %s\n"ANSI_COLOR_RESET, server_addr->host);

	while (1) {
		printf(ANSI_COLOR_YELLOW"waiting for connections\n"ANSI_COLOR_RESET);
		client_addr = comm_listen(server_addr, nil);

		if (!client_addr) {

			fprintf(stderr, ANSI_COLOR_RED"client_addr is null\n"ANSI_COLOR_RESET);

			break;
		}
		printf(ANSI_COLOR_GREEN"opened connection for %s\n"ANSI_COLOR_RESET, client_addr->host);

		childPID = fork();

		if (childPID > 0) {

			// parent

			printf(ANSI_COLOR_GREEN"worker %d created for %s\n"ANSI_COLOR_RESET, childPID, client_addr->host);

		} else {
			if (childPID == -1) {
				fprintf(stderr, ANSI_COLOR_RED "Could not fork\n" ANSI_COLOR_RESET);
				abort();
			}

			while (1) {
				// si no manda nada cuelga aca
				command = comm_receive_data(server_addr, client_addr, nil);
				printf(ANSI_COLOR_CYAN"%s says %s\n"ANSI_COLOR_RESET, client_addr->host, command);

				comm_send_data("hola", 4, server_addr, client_addr, err);

				if (err->code) {

					fprintf(stderr, ANSI_COLOR_RED "error: %d\tmsg:%s\n" ANSI_COLOR_RESET, err->code, err->msg);

				} else {

					printf(ANSI_COLOR_GREEN"data sent successfully: (%s)\n"ANSI_COLOR_RESET, err->msg);

				}
			}

			

			// process data
			// report to msq
			// ask SQL
			// respond
			// 
			
			exit(0);
		}
	}

}


int main(int argc, char **argv) {
	
	
	server_config_t *config;
	char *config_file_opt;
	

	config = NEW(server_config_t);
	
	config_file_opt = process_arguments(argc, argv);

	if (config_file_opt)
		load_configuration(config_file_opt, config);
	else
		load_configuration(DEFAULT_CONFIG_FILE, config);

	printf("connection~>%s\tport~>%d\n", config->connection_queue, config->port);

	listen_connections(config);

	// while (incoming_connections_loop() > 0);

	return 0;
}