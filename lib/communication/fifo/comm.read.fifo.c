#include <comm.receive.api.h>
#include <communication.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/select.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
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
			error->code = 2001;
			error->msg = "FIFO creation failed";
			return;
		}
	}

	error->code = 0;
	error->msg = "Listen Sucessful";

	conn->connection_file = (char*)malloc(input_fifo_len+1);
	strcpy(conn->connection_file, input_fifo);
	conn->state = CONNECTION_STATE_IDLE;
	conn->sense = COMMUNICATION_SERVER_CLIENT;

	free(input_fifo);
}

void comm_accept(connection_t *conn, comm_error_t *error) {

	int fd;
	size_t read_bytes = 0, r_bytes = 0;
	char *buffer, *backup;
	char *request_fifo, *response_fifo;
	size_t request_fifo_len = 0, response_fifo_len = 0;

	comm_addr_t *client_aux;

	if ( (fd = open(conn->connection_file, O_RDONLY)) < 0 ) {
		fprintf(stderr, ANSI_COLOR_RED"%d: open failed %s\n"ANSI_COLOR_RESET, __LINE__, conn->connection_file);
		// fill error;
		ERROR("cant open %s", conn->connection_file);
		error->code = 2002;
		error->msg = "Opening connection file failed";
		return;
	}

	buffer = (char*)malloc(2048);
	memset(buffer, '\0', 2048);

	// read one by one
	do {
		read(fd, buffer+read_bytes, 1);
	} while (*(buffer+read_bytes++) != '\0');
	// wait for close on the other end

	close(fd);

	// hay que escribir el url fd://anon9137
	
	client_aux = NEW(comm_addr_t);

	if (address_from_url(buffer, client_aux)) {
		fprintf(stderr, ANSI_COLOR_RED"%s:%d %s is an invalid address\n"ANSI_COLOR_RESET, __FILE__, __LINE__, buffer);
		ERROR("%s is an invalid address",buffer);
		error->code = 2003;
		error->msg = "Threeway handshake failed. Invalid Address";
		return;
	}

	// request_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(client->host)+strlen(FIFO_REQUEST_EXTENSION)+strlen(FIFO_EXTENSION);
	// armo $CLI.$SRV.fifo
	request_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(client_aux->host)+1+strlen(conn->server_addr->host)+strlen(FIFO_EXTENSION);
	request_fifo = (char*)malloc(request_fifo_len+1);
	request_fifo_len = sprintf(request_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, client_aux->host, conn->server_addr->host, FIFO_EXTENSION);
	request_fifo[request_fifo_len] = '\0';

	if ( mkfifo(request_fifo, FIFO_PERMS) < 0 ) {

		fprintf(stderr, ANSI_COLOR_RED"%d: %s fifo failed creation err:%d msg:%s\n"ANSI_COLOR_RESET, __LINE__, request_fifo, errno, strerror(errno));
		ERROR("%s fifo failed creation err:%d msg:%s", request_fifo, errno, strerror(errno));
		error->code = 2004;
		error->msg = "Request FIFO creation failed";
		return;
	}

	// armo $SRV.$CLI.fifo
	response_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(client_aux->host)+1+strlen(conn->server_addr->host)+strlen(FIFO_EXTENSION);
	response_fifo = (char*)malloc(response_fifo_len+1);
	response_fifo_len = sprintf(response_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, client_aux->host, FIFO_EXTENSION);
	response_fifo[response_fifo_len] = '\0';

	if ( (fd = open(response_fifo, O_WRONLY)) < 0 ) {
		fprintf(stderr, ANSI_COLOR_RED"%d: %s open failed err: %d msg: %s\n"ANSI_COLOR_RESET, __LINE__, response_fifo, errno, strerror(errno));
		abort();
		// rellenar el error
		ERROR("%s open failed err: %d msg: %s", response_fifo, errno, strerror(errno));
		error->code = 2005;
		error->msg = "Response FIFO open failed";
		return; 
	}

	write_one_by_one(fd, buffer, read_bytes+1);

	// copio mi buffer
	backup = strdup(buffer);

	conn->req_fd = fd;

	if ( (fd = open(request_fifo, O_RDONLY)) < 0 ) {
		fprintf(stderr, ANSI_COLOR_RED"%d: open failed %s\n"ANSI_COLOR_RESET, __LINE__, conn->connection_file);
		ERROR("open failed %s",conn->connection_file);
		error->code = 2006;
		error->msg = "Request FIFO open failed";
		return;
	}

	memset(buffer, '\0', 2048);

	r_bytes = read_bytes;
	read_bytes = 0;

	// read one by one
	do {
		read(fd, buffer+read_bytes, 1);
	} while ( read_bytes < r_bytes && *(buffer+read_bytes++) != '\0');

	if ( strcmp(buffer, backup) != 0) {

		fprintf(stderr, ANSI_COLOR_RED"%d: Authentication failed got %s\n"ANSI_COLOR_RESET, __LINE__, buffer);
		ERROR("Authentication failed got %s",buffer);
		error->code = 2004;
		error->msg = "Threeway handshake failed Authentication";
		return;
	}

	conn->client_addr = NEW(comm_addr_t);

	address_from_url(buffer, conn->client_addr);	

	free(buffer);
	free(backup);

	error->code = 0;
	error->msg = "Accept Sucessful";

	conn->res_fd = fd;
	conn->state = CONNECTION_STATE_OPEN;
	
}

char* comm_receive_data(connection_t *conn, comm_error_t *error) {

	char *buffer, *boundary;
	size_t read_bytes = 0;
	size_t boundary_len = 0;

	buffer = (char*)malloc(2048);
	memset(buffer, '\0', 2048);

	boundary = (char*)malloc(128);
	memset(boundary, '\0', 128);

	// get boundary
	do {
		read(conn->res_fd, boundary+read_bytes, 1);
	} while (*(boundary+read_bytes++) != '_');

	read(conn->res_fd, boundary+read_bytes, 1);

	boundary_len = read_bytes+1;

	read_bytes = 0;

	do {
		read(conn->res_fd, buffer+read_bytes, 1);
		if (*(buffer+read_bytes) == ZERO) {
			read_bytes++;
			read(conn->res_fd, buffer+read_bytes, 1);
		}
		read_bytes++;
	} while (read_bytes <= boundary_len || strcmp(buffer+(read_bytes-boundary_len), boundary) != 0);

	buffer[read_bytes-boundary_len] = '\0';

	free(boundary);

	error->code = 0;
	error->msg = "Receive Data Sucessful";

	return buffer;

}
