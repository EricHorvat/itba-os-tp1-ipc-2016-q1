#include <conn.connection.h>

#include <conn.fifo.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>

#include <file_utils.h>

#define INVALID_ADDRESS 2
#define SERVER_OFFLINE 3
#define SERVER_BUSY 4

comm_error_code_t conn_open(connection_t *conn) {

	size_t conn_file_len = 0, request_fifo_len;
	char *request_fifo;
	int fd;

	if (!conn)
		return -1;

	conn->state = CONNECTION_STATE_CLOSED;

	if (!conn->client_addr->valid || !conn->server_addr->valid ) {
		return INVALID_ADDRESS;
	}

	conn_file_len = strlen(FIFO_PATH_PREFIX)+strlen(conn->server_addr->host)+strlen(FIFO_INPUT_EXTENSION)+strlen(FIFO_EXTENSION);
	conn->connection_file = malloc(conn_file_len+1);

	conn_file_len = sprintf(conn->connection_file, "%s%s%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, FIFO_INPUT_EXTENSION, FIFO_EXTENSION);
	conn->connection_file[conn_file_len] = '\0';

	if (!exists(conn->connection_file)) {
		return SERVER_OFFLINE;
	}

	request_fifo_len = strlen(conn->client_addr->url);
	request_fifo = malloc(request_fifo_len+1);

	request_fifo_len = sprintf(request_fifo, "%s", conn->client_addr->url);
	request_fifo[request_fifo_len] = '\0';

	if ( (fd = open(conn->connection_file, O_WRONLY)) == -1) {
		return SERVER_BUSY;
	}

	write_one_by_one(fd, (void*)request_fifo, request_fifo_len);

	close(fd);

	conn->state = CONNECTION_STATE_OPEN;

	return 0;

}

comm_error_code_t conn_close(connection_t *conn) {

	char *request_fifo, *response_fifo;
	size_t request_fifo_len, response_fifo_len;	

	if (!conn)
		return -1;

	request_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(conn->client_addr->host)+strlen(conn->server_addr->host)+strlen(FIFO_EXTENSION);
	request_fifo = malloc(request_fifo_len+2);

	response_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(conn->server_addr->host)+strlen(conn->client_addr->host)+strlen(FIFO_EXTENSION);
	response_fifo = malloc(response_fifo_len+2);

	request_fifo_len = sprintf(request_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->client_addr->host, conn->server_addr->host, FIFO_EXTENSION);
	request_fifo[request_fifo_len] = '\0';

	response_fifo_len = sprintf(response_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, conn->client_addr->host, FIFO_EXTENSION);
	response_fifo[response_fifo_len] = '\0';

	if (exists(request_fifo))
		unlink(request_fifo);

	if (exists(response_fifo))
		unlink(response_fifo);

	conn->state = CONNECTION_STATE_CLOSED;

	return 0;

}