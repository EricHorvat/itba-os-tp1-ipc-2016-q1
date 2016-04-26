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
static int incoming_connections_loop();

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

static int incoming_connections_loop() {

	printf("> Starting loop\n");

	char *fifo = "/tmp/google.com.in.fifo";//INCOMING_CONNECTIONS_FIFO;
	char *buffer, *req_buffer, *host;
	size_t read_bytes = 0, req_read_bytes = 0;
	int fd = -1, req_fd = -1, res_fd = -1;

	buffer = (char*)malloc(2048);
	req_buffer = (char*)malloc(2048);
	host = (char*)malloc(2048);

	memset(buffer, ZERO, 2048);
	memset(req_buffer, ZERO, 2048);
	memset(host, ZERO, 2048);

	while (1) {

		


		// is file desc open?
		if (fcntl(fd, F_GETFL) < 0) {

			printf("opening fd\n");

			if (access(fifo, F_OK) == -1) {
				// file does not exist ==> create file
				if (mkfifo(fifo, 0666) == -1) {
					printf("creating %s failed err: %d errno: %s\n", fifo, errno, strerror(errno));
					return 3;
				}
			}

			if ( (fd = open(fifo, O_RDONLY)) == -1) {

				printf("FIFO[%s] could not be opened\n", fifo);
				unlink(fifo);
				return 3;

			}
		}

		// incoming connections fifo is created
		// listen

		do {
			read(fd, buffer+read_bytes, 1);
		} while ( *(buffer+read_bytes) != '\n' && *(buffer+read_bytes) != '\0' && ++read_bytes);

		// if buffer was closed
		if (*(buffer+read_bytes) == '\0') {
			close(fd);
		}

		buffer[read_bytes] = '\0';

		strncpy(host, buffer, strlen(buffer)-9);

		// en buffer esta el hostname del que me quiere hablar
		printf("%s wants to send something\n", buffer);

		int attempts = 0;


		while (attempts++ < 10 && access(buffer, F_OK) == -1) {
		    usleep(500);
		    printf("%s does not exist\n", buffer);
		}

		// if (access(buffer, F_OK) == -1) {
			// printf("%s does not exist\n", buffer);
				// file does not exist ==> create file
			// if (mkfifo(buffer, 0666) == -1) {
				// printf("mkfifo failed err: %d msg: %s\n", errno, strerror(errno));
				// return 3;
			// }
		// }

		printf("opening request fifo %s\n", buffer);
		if ( (req_fd = open(buffer, O_RDONLY)) == -1) {

			printf("Request FIFO could not be opened\n");
			unlink(fifo);
			return 3;

		}

		printf("reading request from %s\n", buffer);
		do {
			read(req_fd, req_buffer+req_read_bytes, 1);
		} while ( *(req_buffer+req_read_bytes) != '\n' && *(req_buffer+req_read_bytes) != '\0' && ++req_read_bytes);

		req_buffer[req_read_bytes] = 0;

		close(req_fd);
		unlink(buffer);

		printf("%s says %s\n", host+5, req_buffer);


		strcat(host, ".res.fifo");

		printf("creating fifo: %s\n", host);

		if (access(host, F_OK) == -1) {
			printf("%s was not created\n", host);
			// file does not exist ==> create file
			if (mkfifo(host, 0666) == -1) {
				printf("mkfifo failed err: %d msg: %s\n", errno, strerror(errno));
				unlink(host);
				return 3;
			}
			printf("%s created successfully\n", host);
		}

		if ( (res_fd = open(host, O_WRONLY)) == -1) {

			printf("Response FIFO could not be opened\n");
			unlink(host);
			return 3;

		}

		write(res_fd, "afasdsads", 4);

		close(res_fd);

		// unlink(host);

		res_fd = req_fd = -1;
		buffer[0] = req_buffer[0] = '\0';
		memset(host, '\0', 100);
		read_bytes = req_read_bytes = 0;
	}

}

static void listen_connections(server_config_t *config) {

	comm_addr_t *server_addr, *client_addr;
	pid_t childPID;
	char *command;

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
				command = comm_receive_data(client_addr, nil);
				printf(ANSI_COLOR_CYAN"%s says %s\n"ANSI_COLOR_RESET, client_addr->host, command);
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