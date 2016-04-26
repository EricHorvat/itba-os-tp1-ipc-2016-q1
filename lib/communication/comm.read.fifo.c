#include <comm.receive.api.h>
#include <comm.addr.h>
#include <communication.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
// https://gist.github.com/jbenet/1087739
#include <time.h>
#include <sys/time.h>
#include <pthread.h>


#include <file_utils.h>
#include <utils.h>
#include <comm.fifo.h>

typedef struct {
	comm_callback_t cb;
	comm_addr_t* origin;
	comm_addr_t* endpoint;
} comm_thread_info_t;

typedef struct {
	int fd;
	char *fifo;
	bool success;
} comm_data_writer_ret_t;

typedef void* (*pthread_func_t)(void* data);

static unsigned int TIMEOUT = 10;

/**
 * [set_timeout description]
 * @param t timeout to set
 */
/*void set_timeout(unsigned int t) {
	TIMEOUT = t;
}*/

comm_addr_t* comm_listen(comm_addr_t *server, comm_error_t *error) {

	char *input_fifo;
	size_t input_fifo_len = 0;
	int fd;
	size_t read_bytes = 0;
	char *buffer;
	comm_addr_t *client;

	input_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(server->host)+strlen(FIFO_INPUT_EXTENSION)+strlen(FIFO_EXTENSION);
	input_fifo = (char*)malloc(input_fifo_len+1);

	input_fifo_len = sprintf(input_fifo, "%s%s%s%s", FIFO_PATH_PREFIX, server->host, FIFO_INPUT_EXTENSION, FIFO_EXTENSION);
	input_fifo[input_fifo_len] = '\0';

	if (!exists(input_fifo)) {

		if (mkfifo(input_fifo, FIFO_PERMS) == -1) {
			fprintf(stderr, ANSI_COLOR_RED"mkfifo failed\n"ANSI_COLOR_RESET);
			return nil;
		}
	}

	if ( (fd = open(input_fifo, O_RDONLY)) == -1 ) {
		fprintf(stderr, ANSI_COLOR_RED"open failed\n"ANSI_COLOR_RESET);
		// fill error;
		return nil;
	}

	buffer = (char*)malloc(2048);
	memset(buffer, '\0', 2048);

	// read one by one
	do {
		read(fd, buffer+read_bytes, 1);
	} while (*(buffer+read_bytes++) != '\0');
	// wait for close on the other end

	client = NEW(comm_addr_t);
	// hay que escribir el url fd://anon9137

	address_from_url(buffer, client);

	return client;
	
}

char* comm_receive_data(comm_addr_t *client, comm_error_t *error) {

	char *request_fifo;
	size_t request_fifo_len = 0, read_bytes = 0;
	int fd;
	char *buffer;

	request_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(client->host)+strlen(FIFO_REQUEST_EXTENSION)+strlen(FIFO_EXTENSION);

	request_fifo = (char*)malloc(request_fifo_len+1);

	request_fifo_len = sprintf(request_fifo, "%s%s%s%s", FIFO_PATH_PREFIX, client->host, FIFO_REQUEST_EXTENSION, FIFO_EXTENSION);
	request_fifo[request_fifo_len] = '\0';

	if (!exists(request_fifo)) {
		fprintf(stderr, ANSI_COLOR_RED"file does not exist\n"ANSI_COLOR_RESET);
		// OJO esta linea
		while (!exists(request_fifo)); // OJO esta linea
		// OJO esta linea
	}

	if ( (fd = open(request_fifo, O_RDONLY)) == -1 ) {
		fprintf(stderr, ANSI_COLOR_RED"open failed\n"ANSI_COLOR_RESET);
		abort();
		// rellenar el error

		return nil;
	}

	buffer = (char*)malloc(2048);
	memset(buffer, '\0', 2048);

	do {
		read(fd, buffer+read_bytes, 1);
	} while (*(buffer+read_bytes++) != '\0');

	return buffer;

}

