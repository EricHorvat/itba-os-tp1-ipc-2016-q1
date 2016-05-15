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

#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>


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

	int fd;
	struct sockaddr_in * serv_addr;
	
	serv_addr = malloc(sizeof(struct sockaddr_in));
	// memset(serv_addr,0,sizeof(struct sockaddr_in));
	serv_addr->sin_family = AF_INET;
	serv_addr->sin_addr.s_addr = INADDR_ANY;

	serv_addr->sin_port = conn->server_addr->extra->port;


	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)	{
		ERROR("cant create socket");
		error->code = ERR_SOCKET_NOT_CREATED;
		error->msg = "cant create socket";
		return;
	}

	if ((bind(fd, (struct sockaddr *) serv_addr, sizeof(struct sockaddr_in))) < 0) {
		ERROR("cant bind socket");
		error->code = ERR_SOCKET_NOT_BINDED;
		error->msg = "cant binded socket";
		return;
	}
	if ((listen(fd,MAX_SOCKETS_CONNECTIONS)) < 0) {
		ERROR("cant listen socket");
		error->code = ERR_SOCKET_NOT_LISTENED;
		error->msg = "cant listen socket";
		return;
	}

	INFO("host: %s", conn->server_addr->host);

	INFO("fd: %d", fd);

	/*HORIPILANTE**/
	conn->connection_file = (char*)malloc( 10 );
	bzero(conn->connection_file, 10);
	sprintf(conn->connection_file, "%d", fd);
	/**/

	error->code = 0;
	error->msg= "OK";

	conn->state = CONNECTION_STATE_IDLE;
}

void comm_accept(connection_t *conn, comm_error_t *error) {

	
	int fd, read_bytes = 0, at;
	char * buffer = (char*)malloc(BUFFER_LENGTH);

	struct sockaddr_in client_addr;
	socklen_t clilen;

	memset(buffer, ZERO, BUFFER_LENGTH);

	INFO("connection_file: %s", conn->connection_file);

	at = atoi(conn->connection_file);

	INFO("at: %d", at);

	clilen = sizeof(client_addr);
	if ((fd = accept(at/*FEO*/, (struct sockaddr *) &client_addr, &clilen)) < 0) {
		ERROR("cant accept socket");
		error->code = ERR_SOCKET_NOT_ACCEPTED;
		error->msg = "cant accepted socket";
	}
	INFO("socket accepted");

	// read one by one
	do {
		read(fd, buffer+read_bytes, 1);
	} while ( *(buffer+read_bytes++) != '\0');

	INFO("buffer: %s", buffer);

	conn->client_addr = NEW(comm_addr_t);

	if (address_from_url(buffer, conn->client_addr)) {
		ERROR("address cant be created");
		error->code = ERR_SOCKET_ADDRESS_NOT_CREATED;
		error->msg = "address cant be created";
	}

	// INFO("writing");
	// write_one_by_one(fd, buffer, strlen(buffer) );
	// INFO("ended writing");
	conn->res_fd = fd;
	conn->req_fd = fd;
	conn->state = CONNECTION_STATE_OPEN;

	error->code = 0;
	error->msg= "OK";

}

char* comm_receive_data(connection_t *conn, comm_error_t *error) {

	//char *request_fifo;
	//size_t request_fifo_len = 0, read_bytes = 0;
	//int fd;
	char *buffer;
	size_t read_bytes = 0;


	buffer = (char*)malloc(BUFFER_LENGTH);
	memset(buffer, ZERO, BUFFER_LENGTH);

	INFO("reading from %d", conn->res_fd);

	do {
		read(conn->res_fd, buffer+read_bytes, 1);
	} while (*(buffer+read_bytes++) != '\0');
	//close(fd);
	//
	INFO("read from %d", conn->res_fd);
	INFO("read [%s]", buffer);

	error->code = 0;
	error->msg= "OK";

	return buffer;

}
