#include <comm.send.api.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include <file_utils.h>
#include <utils.h>
#include <comm.fifo.h>

/**

	Typedefs

**/

typedef struct {
	comm_callback_t cb;
	connection_t*   conn;
} comm_thread_info_t;

typedef struct {
	int   fd;
	char* fifo;
	boolean  success;
} comm_data_writer_ret_t;

typedef void* (*pthread_func_t)(void* data);

/**

	Prototypes

**/

static void* data_listener(void*);

pthread_mutex_t lock;
boolean            mutex_init = no;

static void* data_listener(void* data) {

	comm_error_t*       error;
	comm_thread_info_t* info;
	char *              buffer, *boundary;
	size_t              read_bytes   = 0;
	size_t              boundary_len = 0;

	info = (comm_thread_info_t*)data;

	error = NEW(comm_error_t);

	buffer = (char*)malloc(BUFFER_LENGTH);
	memset(buffer, '\0', BUFFER_LENGTH);

	boundary = (char*)malloc(BOUNDARY_LENGTH);
	memset(boundary, '\0', BOUNDARY_LENGTH);

	pthread_mutex_lock(&lock);
	// get boundary
	do {
		read(info->conn->res_fd, boundary + read_bytes, 1);
	} while (*(boundary + read_bytes++) != '_');

	read(info->conn->res_fd, boundary + read_bytes, 1);

	boundary_len = read_bytes + 1;

	read_bytes = 0;

	do {
		read(info->conn->res_fd, buffer + read_bytes, 1);
		if (*(buffer + read_bytes) == ZERO) {
			read_bytes++;
			read(info->conn->res_fd, buffer + read_bytes, 1);
		}
		read_bytes++;
	} while (read_bytes <= boundary_len || strcmp(buffer + (read_bytes - boundary_len), boundary) != 0);

	pthread_mutex_unlock(&lock);

	buffer[read_bytes - boundary_len] = '\0';

	free(boundary);

	error->code = NO_COMM_ERROR;
	error->msg  = "Receive Data Sucessful";

	info->cb(error, info->conn, buffer);

	pthread_exit(NULL);

	return nil;
}

void comm_send_data(void* data, size_t size, connection_t* conn, comm_error_t* error) {

	char* boundary;

	if (!error)
		error = NEW(comm_error_t);

	boundary = gen_boundary();
	flock(conn->req_fd, LOCK_EX);
	write_one_by_one_without_zero(conn->req_fd, boundary, strlen(boundary));
	write_one_by_one_without_zero(conn->req_fd, data, size);
	write_one_by_one_without_zero(conn->req_fd, boundary, strlen(boundary));
	flock(conn->req_fd, LOCK_UN);

	error->code = NO_COMM_ERROR;
	error->msg  = "OK";
}

void comm_send_data_async(void* data, size_t size, connection_t* conn, comm_callback_t cb) {

	int                 pthread_ret;
	pthread_t           listener;
	comm_error_t*       err;
	comm_thread_info_t* thread_arg;
	char*               boundary;

	boundary = gen_boundary();
	INFO("locking fd(%d)", conn->req_fd);
	flock(conn->req_fd, LOCK_EX);
	write_one_by_one_without_zero(conn->req_fd, boundary, strlen(boundary));
	write_one_by_one_without_zero(conn->req_fd, data, size);
	write_one_by_one_without_zero(conn->req_fd, boundary, strlen(boundary));
	flock(conn->req_fd, LOCK_UN);
	INFO("unlocking fd(%d)", conn->req_fd);

	thread_arg = NEW(comm_thread_info_t);

	// fill in thread arguments
	thread_arg->cb   = cb;
	thread_arg->conn = conn;

	if (!mutex_init) {
		if (pthread_mutex_init(&lock, NULL) != 0) {
			printf("\n mutex init failed\n");
			err       = NEW(comm_error_t);
			err->code = 3;
			err->msg  = "mutex init failed";
			return cb(err, conn, null);
		}
		mutex_init = yes;
	}

	if ((pthread_ret = pthread_create(&listener, NULL, data_listener, (void*)thread_arg))) {
		ERROR("pthread create returned %d\n", pthread_ret);
	}
}

boolean is_connection_open(connection_t* conn) {
	return conn->state == CONNECTION_STATE_OPEN;
}

boolean is_connection_closed(connection_t* conn) {
	return conn->state == CONNECTION_STATE_CLOSED;
}
