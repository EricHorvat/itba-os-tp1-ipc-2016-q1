#include <comm.connection.api.h>
#include <comm.addr.api.h>
#include <comm.fifo.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>

#include <file_utils.h>

comm_error_code_t connection_open(connection_t* conn) {

	size_t conn_file_len = 0, request_fifo_len, response_fifo_len;
	char * request_fifo, *response_fifo;
	int    fd;
	char*  buffer;
	size_t read_bytes = 0, url_len;

	if (!conn)
		return ERR_NULL_CONNECTION;

	conn->state = CONNECTION_STATE_CLOSED;

	if (!conn->client_addr->valid || !conn->server_addr->valid) {
		return ERR_INVALID_ADDRESS;
	}

	url_len = strlen(conn->client_addr->url);

	// 3-way handshake hell

	// creo $SRV.$CLI.fifo
	response_fifo_len                = strlen(FIFO_PATH_PREFIX) + strlen(conn->client_addr->host) + 1 + strlen(conn->server_addr->host) + strlen(FIFO_EXTENSION);
	response_fifo                    = (char*)malloc(response_fifo_len + 1);
	response_fifo_len                = sprintf(response_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, conn->client_addr->host, FIFO_EXTENSION);
	response_fifo[response_fifo_len] = ZERO;

	if (mkfifo(response_fifo, FIFO_PERMS) < 0) {

		fprintf(stderr, ANSI_COLOR_RED "%s fifo failed creation err:%d msg:%s\n" ANSI_COLOR_RESET, response_fifo, errno, strerror(errno));
		return ERR_FIFO_FAIL_CONNECTION;
	}

	// armo string $CLI.$SRV.fifo
	request_fifo_len               = strlen(FIFO_PATH_PREFIX) + strlen(conn->client_addr->host) + 1 + strlen(conn->server_addr->host) + strlen(FIFO_EXTENSION);
	request_fifo                   = (char*)malloc(request_fifo_len + 1);
	request_fifo_len               = sprintf(request_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->client_addr->host, conn->server_addr->host, FIFO_EXTENSION);
	request_fifo[request_fifo_len] = ZERO;

	// armo connection file
	conn_file_len                        = strlen(FIFO_PATH_PREFIX) + strlen(conn->server_addr->host) + strlen(FIFO_INPUT_EXTENSION) + strlen(FIFO_EXTENSION);
	conn->connection_file                = malloc(conn_file_len + 1);
	conn_file_len                        = sprintf(conn->connection_file, "%s%s%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, FIFO_INPUT_EXTENSION, FIFO_EXTENSION);
	conn->connection_file[conn_file_len] = '\0';

	// me fijo si existe
	if (!exists(conn->connection_file)) {
		return ERR_SERVER_OFFLINE;
	}

	if ((fd = open(conn->connection_file, O_WRONLY)) < 0) {
		return ERR_SERVER_BUSY;
	}

	write_one_by_one(fd, (void*)conn->client_addr->url, url_len);
	close(fd);

	//busy_wait_file_exists(request_fifo);

	// bloquea hasta que repondan por aca
	if ((fd = open(response_fifo, O_RDONLY)) < 0) {

		fprintf(stderr, ANSI_COLOR_RED "%s open failed err:%d msg:%s\n" ANSI_COLOR_RESET, response_fifo, errno, strerror(errno));
		return ERR_FIFO_CANT_OPEN_RESPONSE;
	}

	conn->res_fd = fd;

	// escuchamos el ack del srv
	buffer = (char*)malloc(url_len + 1);
	do {
		read(fd, buffer + read_bytes, 1);
	} while (read_bytes < url_len && *(buffer + read_bytes++) != ZERO);

	// comparamos para evitar corrupcion de datos
	if (strcmp(buffer, conn->client_addr->url) != 0) {
		fprintf(stderr, ANSI_COLOR_RED "Authentication failed got %s\n" ANSI_COLOR_RESET, buffer);
		return ERR_CORRUPT_DATA;
	}

	INFO("Authentication succeeded");

	// mandamos nuestro ok
	if ((fd = open(request_fifo, O_WRONLY)) < 0) {
		fprintf(stderr, ANSI_COLOR_RED "%s open failed err:%d msg:%s\n" ANSI_COLOR_RESET, request_fifo, errno, strerror(errno));
		return ERR_FIFO_CANT_OPEN_REQUEST;
	}

	write_one_by_one(fd, (void*)conn->client_addr->url, url_len + 1);

	// ya estoy conectado

	conn->req_fd = fd;
	conn->state  = CONNECTION_STATE_OPEN;
	conn->sense  = COMMUNICATION_CLIENT_SERVER;

	return CONNECTION_OK;
}

comm_error_code_t connection_close(connection_t* conn) {

	char * request_fifo, *response_fifo;
	size_t request_fifo_len, response_fifo_len;

	if (!conn)
		return ERR_NULL_CONNECTION;

	request_fifo_len = strlen(FIFO_PATH_PREFIX) + strlen(conn->client_addr->host) + strlen(conn->server_addr->host) + strlen(FIFO_EXTENSION);
	request_fifo     = malloc(request_fifo_len + 2);

	response_fifo_len = strlen(FIFO_PATH_PREFIX) + strlen(conn->server_addr->host) + strlen(conn->client_addr->host) + strlen(FIFO_EXTENSION);
	response_fifo     = malloc(response_fifo_len + 2);

	request_fifo_len               = sprintf(request_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->client_addr->host, conn->server_addr->host, FIFO_EXTENSION);
	request_fifo[request_fifo_len] = '\0';

	response_fifo_len                = sprintf(response_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, conn->client_addr->host, FIFO_EXTENSION);
	response_fifo[response_fifo_len] = '\0';

	close(conn->req_fd);

	conn->res_fd = 0;
	conn->req_fd = 0;

	if (exists(request_fifo))
		unlink(request_fifo);

	if (exists(response_fifo))
		unlink(response_fifo);

	conn->state = CONNECTION_STATE_CLOSED;

	return CONNECTION_OK;
}
