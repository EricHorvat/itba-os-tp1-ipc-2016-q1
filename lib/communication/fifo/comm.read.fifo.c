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
			return;
		}
	}

	conn->connection_file = (char*)malloc(input_fifo_len+1);
	strcpy(conn->connection_file, input_fifo);
	conn->state = CONNECTION_STATE_IDLE;
	conn->sense = COMMUNICATION_SERVER_CLIENT;

	free(input_fifo);
}

static void info(char* info) {
	printf(ANSI_COLOR_CYAN"%s:%d %s\n"ANSI_COLOR_RESET, __FILE__, __LINE__, info);
}

void comm_accept(connection_t *conn, comm_error_t *error) {

	int fd;
	size_t read_bytes = 0, r_bytes = 0;
	char *buffer, *backup;
	char *request_fifo, *response_fifo;
	size_t request_fifo_len = 0, response_fifo_len = 0;

	comm_addr_t *client_aux;

	info("will open connection file");

	if ( (fd = open(conn->connection_file, O_RDONLY)) < 0 ) {
		fprintf(stderr, ANSI_COLOR_RED"%d: open failed %s\n"ANSI_COLOR_RESET, __LINE__, conn->connection_file);
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

	info("read from conenction file");

	close(fd);

	// hay que escribir el url fd://anon9137
	
	client_aux = NEW(comm_addr_t);

	if (address_from_url(buffer, client_aux)) {
		fprintf(stderr, ANSI_COLOR_RED"%s:%d %s is an invalid address\n"ANSI_COLOR_RESET, __FILE__, __LINE__, buffer);
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
		return;
	}

	info("request mkfifo built ok");

	// armo $SRV.$CLI.fifo
	response_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(client_aux->host)+1+strlen(conn->server_addr->host)+strlen(FIFO_EXTENSION);
	response_fifo = (char*)malloc(response_fifo_len+1);
	response_fifo_len = sprintf(response_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, client_aux->host, FIFO_EXTENSION);
	response_fifo[response_fifo_len] = '\0';


	info("writing response fifo");

	if ( (fd = open(response_fifo, O_WRONLY)) < 0 ) {
		fprintf(stderr, ANSI_COLOR_RED"%d: %s open failed err: %d msg: %s\n"ANSI_COLOR_RESET, __LINE__, response_fifo, errno, strerror(errno));
		abort();
		// rellenar el error

		return; 
	}

	info("opened response fifo");

	write_one_by_one(fd, buffer, read_bytes+1);

	// copio mi buffer
	backup = strdup(buffer);

	conn->req_fd = fd;

	info("opening request fifo");

	if ( (fd = open(request_fifo, O_RDONLY)) < 0 ) {
		fprintf(stderr, ANSI_COLOR_RED"%d: open failed %s\n"ANSI_COLOR_RESET, __LINE__, conn->connection_file);
		// fill error;
		return;
	}

	info("opened request_fifo");

	memset(buffer, '\0', 2048);

	r_bytes = read_bytes;
	read_bytes = 0;


	info("bout to read");
	// read one by one
	do {
		read(fd, buffer+read_bytes, 1);
	} while ( read_bytes < r_bytes && *(buffer+read_bytes++) != '\0');

	info("read ok");

	if ( strcmp(buffer, backup) != 0) {

		fprintf(stderr, ANSI_COLOR_RED"%d: Authentication failed got %s\n"ANSI_COLOR_RESET, __LINE__, buffer);
		return;

	}

	info("auth ok");


	conn->client_addr = NEW(comm_addr_t);

	address_from_url(buffer, conn->client_addr);	

	free(buffer);
	free(backup);

	printf(ANSI_COLOR_GREEN"soy el #1\n"ANSI_COLOR_RESET);

	conn->res_fd = fd;
	conn->state = CONNECTION_STATE_OPEN;
	
}

char* comm_receive_data(connection_t *conn, comm_sense_t sense, comm_error_t *error) {

	//char *request_fifo;
	//size_t request_fifo_len = 0, read_bytes = 0;
	//int fd;
	char *buffer;
	size_t read_bytes = 0;

	// request_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(client->host)+strlen(FIFO_REQUEST_EXTENSION)+strlen(FIFO_EXTENSION);
	//request_fifo_len = strlen(FIFO_PATH_PREFIX)+strlen(conn->client_addr->host)+strlen(conn->server_addr->host)+strlen(FIFO_EXTENSION);
	//request_fifo = (char*)malloc(request_fifo_len+2);

	// request_fifo_len = sprintf(request_fifo, "%s%s%s%s", FIFO_PATH_PREFIX, client->host, FIFO_REQUEST_EXTENSION, FIFO_EXTENSION);
	//if (sense == COMMUNICATION_CLIENT_SERVER)
	//	request_fifo_len = sprintf(request_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->client_addr->host, conn->server_addr->host, FIFO_EXTENSION);
	//else
	//	request_fifo_len = sprintf(request_fifo, "%s%s.%s%s", FIFO_PATH_PREFIX, conn->server_addr->host, conn->client_addr->host, FIFO_EXTENSION);
	//request_fifo[request_fifo_len] = '\0';

	//if (!exists(request_fifo)) {
	//	fprintf(stderr, ANSI_COLOR_RED"file [%s] does not exist\n"ANSI_COLOR_RESET, request_fifo);
		


		// OJO esta linea
	//	while (!exists(request_fifo)) { // OJO esta linea
	//		sleep(1); // OJO esta linea
	//	} // OJO esta linea
		// OJO esta linea
	//	printf(ANSI_COLOR_GREEN"file [%s] now exists\n"ANSI_COLOR_RESET, request_fifo);
	//}

	//printf(ANSI_COLOR_CYAN"%d will open %s\n"ANSI_COLOR_RESET, getpid(), request_fifo);
	
	//if ( (fd = open(request_fifo, O_RDONLY)) < 0 ) {
	//	printf(ANSI_COLOR_RED"open failed\n"ANSI_COLOR_RESET);
	//	fprintf(stderr, ANSI_COLOR_RED"open failed\n"ANSI_COLOR_RESET);
	//	abort();
		// rellenar el error

	//	return nil; 
	//}
	//printf(ANSI_COLOR_CYAN"opened %s\n"ANSI_COLOR_RESET, request_fifo);

	buffer = (char*)malloc(2048);
	memset(buffer, '\0', 2048);

	do {
		read(conn->res_fd, buffer+read_bytes, 1);
	} while (*(buffer+read_bytes++) != '\0');
	//close(fd);

	return buffer;

}
