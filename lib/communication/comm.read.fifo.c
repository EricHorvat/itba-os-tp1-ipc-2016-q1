#include <comm.receive.api.h>
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

void comm_listen(connection_t *conn, comm_error_t *error) {

	char *input_fifo;
	size_t input_fifo_len = 0;

	input_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(conn->server_addr->host)+strlen(FIFO_INPUT_EXTENSION)+strlen(FIFO_EXTENSION);
	input_fifo = (char*)malloc(input_fifo_len+1);

	input_fifo_len = sprintf(input_fifo, "%s%s%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, FIFO_INPUT_EXTENSION, FIFO_EXTENSION);
	input_fifo[input_fifo_len] = '\0';


	if (!exists(input_fifo)) {

		if (mkfifo(input_fifo, FIFO_PERMS) < 0) {
			fprintf(stderr, ANSI_COLOR_RED"mkfifo failed\n"ANSI_COLOR_RESET);
			return;
		}
	}

	conn->connection_file = input_fifo;
	conn->state = CONNECTION_STATE_IDLE;

}

void comm_accept(connection_t *conn, comm_error_t *error) {

	int fd;
	size_t read_bytes = 0;
	char *buffer;

	if ( (fd = open(conn->connection_file, O_RDONLY)) < 0 ) {
		fprintf(stderr, ANSI_COLOR_RED"open failed %s\n"ANSI_COLOR_RESET, conn->connection_file);
		// fill error;
		return;
	}

	buffer = (char*)malloc(2048);
	memset(buffer, '\0', 2048);

	// read one by one
	do {
		read(fd, buffer+read_bytes, 1);
	} while (*(buffer+read_bytes++) != '\0');
	// wait for close on the other end

	// hay que escribir el url fd://anon9137

	conn->client_addr = NEW(comm_addr_t);

	address_from_url(buffer, conn->client_addr);

	conn->state = CONNECTION_STATE_OPEN;
	
}

char* comm_receive_data(connection_t *conn, comm_sense_t sense, comm_error_t *error) {

	char *request_fifo;
	size_t request_fifo_len = 0, read_bytes = 0;
	int fd;
	char *buffer;

	// request_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(client->host)+strlen(FIFO_REQUEST_EXTENSION)+strlen(FIFO_EXTENSION);
	request_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(conn->client_addr->host)+strlen(conn->server_addr->host)+strlen(FIFO_EXTENSION);
	request_fifo = (char*)malloc(request_fifo_len+1);

	// request_fifo_len = sprintf(request_fifo, "%s%s%s%s", FIFO_PATH_PREFIX, client->host, FIFO_REQUEST_EXTENSION, FIFO_EXTENSION);
	if (sense == COMMUNICATION_CLIENT_SERVER)
		request_fifo_len = sprintf(request_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->client_addr->host, conn->server_addr->host, FIFO_EXTENSION);
	else
		request_fifo_len = sprintf(request_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, conn->client_addr->host, FIFO_EXTENSION);
	request_fifo[request_fifo_len] = '\0';

	if (!exists(request_fifo)) {
		fprintf(stderr, ANSI_COLOR_RED"file [%s] does not exist\n"ANSI_COLOR_RESET, request_fifo);
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

