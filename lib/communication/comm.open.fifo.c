#include <communication.h>

#include <comm.fifo.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <file_utils.h>

#define INVALID_ADDRESS 2
#define SERVER_OFFLINE 3
#define SERVER_BUSY 4

comm_error_code_t comm_open(connection_t *comm) {

	size_t conn_file_len = 0, request_fifo_len;
	char *request_fifo;
	int fd;

	if (!comm)
		return -1;

	if (!comm->client_addr->valid || !comm->server_addr->valid ) {
		return INVALID_ADDRESS;
	}

	conn_file_len = strlen(FIFO_PATH_PREFIX)+strlen(comm->server_addr->host)+strlen(FIFO_INPUT_EXTENSION)+strlen(FIFO_EXTENSION);
	comm->connection_file = malloc(conn_file_len+1);

	conn_file_len = sprintf(comm->connection_file, "%s%s%s%s", FIFO_PATH_PREFIX, comm->server_addr->host, FIFO_INPUT_EXTENSION, FIFO_EXTENSION);
	comm->connection_file[conn_file_len] = '\0';

	if (!exists(comm->connection_file)) {
		return SERVER_OFFLINE;
	}

	request_fifo_len = strlen(comm->client_addr->url);
	request_fifo = malloc(request_fifo_len+1);

	request_fifo_len = sprintf(request_fifo, "%s", comm->client_addr->url);
	request_fifo[request_fifo_len] = '\0';

	if ( (fd = open(comm->connection_file, O_WRONLY)) == -1) {
		return SERVER_BUSY;
	}

	write_one_by_one(fd, (void*)request_fifo, request_fifo_len);

	close(fd);

	return 0;

}