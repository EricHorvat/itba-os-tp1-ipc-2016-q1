#include <comm.send.api.h>
#include <communication.h>

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/file.h>
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

/**

	Typedefs

**/

typedef struct {
	comm_callback_t cb;
	connection_t *conn;
} comm_thread_info_t;

typedef struct {
	int fd;
	char *fifo;
	bool success;
} comm_data_writer_ret_t;

typedef void* (*pthread_func_t)(void* data);

/**

	Prototypes

**/

static void *data_listener(void *);


pthread_mutex_t lock;
bool mutex_init = no;

static void *data_listener(void *data) {

	comm_error_t *err;
	//char *fifo;
	//size_t fifo_len;
	size_t read_bytes = 0;
	//int fd;
	char* buffer;
	comm_thread_info_t *info;

	long int self = (long int)pthread_self();

	buffer = (char*)malloc(2048);
	memset(buffer, '\0', 2048);

	info = (comm_thread_info_t*)data;

	err = NEW(comm_error_t);

	pthread_mutex_lock(&lock);

	printf("[thread %ld] about to read %d\n", self, info->conn->res_fd);
	//flock(fd, LOCK_SH);
	do {
		read(info->conn->res_fd, buffer+read_bytes, 1);
	} while (*(buffer+read_bytes++) != '\0');
	//flock(fd, LOCK_UN);
	pthread_mutex_unlock(&lock);
	//close(fd);


	err->code = 0;
	err->msg = "Operacion Exitosa";

	info->cb(err, info->conn, buffer);

	pthread_exit(NULL);

	return nil;

}

void comm_send_data(void *data, size_t size, connection_t *conn, comm_error_t *error) {

	char *boundary;

	if (!error)
		error = NEW(comm_error_t);

	boundary = gen_boundary();
	printf(ANSI_COLOR_CYAN"locking fd(%d)\n"ANSI_COLOR_RESET, conn->req_fd);
	flock(conn->req_fd, LOCK_EX);
	write_one_by_one_without_zero(conn->req_fd, boundary, strlen(boundary));
	write_one_by_one_without_zero(conn->req_fd, data, size);
	write_one_by_one_without_zero(conn->req_fd, boundary, strlen(boundary));
	flock(conn->req_fd, LOCK_UN);
	printf(ANSI_COLOR_CYAN"unlocking fd(%d)\n"ANSI_COLOR_RESET, conn->req_fd);
	error->code = NO_COMM_ERROR;
	error->msg = "Todo OK";
	
}

void comm_send_data_async(void * data, size_t size, connection_t *conn, comm_callback_t cb) {

	int pthread_ret;
	pthread_t listener;
	comm_error_t *err;
	comm_thread_info_t *thread_arg;
	char *boundary;

	boundary = gen_boundary();
	printf(ANSI_COLOR_CYAN"locking fd(%d)\n"ANSI_COLOR_RESET, conn->req_fd);
	flock(conn->req_fd, LOCK_EX);
	write_one_by_one_without_zero(conn->req_fd, boundary, strlen(boundary));
	write_one_by_one_without_zero(conn->req_fd, data, size);
	write_one_by_one_without_zero(conn->req_fd, boundary, strlen(boundary));
	flock(conn->req_fd, LOCK_UN);
	printf(ANSI_COLOR_CYAN"unlocking fd(%d)\n"ANSI_COLOR_RESET, conn->req_fd);

	thread_arg = NEW(comm_thread_info_t);

	// fill in thread arguments
	thread_arg->cb = cb;
	thread_arg->conn = conn;



	if (!mutex_init) {
		if (pthread_mutex_init(&lock, NULL) != 0) {
        	printf("\n mutex init failed\n");
        	err = NEW(comm_error_t);
        	err->code = 3;
        	err->msg = "mutex init failed";
        	return cb(err, conn, null);
        }
        mutex_init = yes;
    }

	if ( (pthread_ret = pthread_create(&listener, NULL, data_listener, (void*)thread_arg)) ) {
		fprintf(stderr, ANSI_COLOR_RED"pthread create returned %d\n"ANSI_COLOR_RED, pthread_ret);
	}

}
