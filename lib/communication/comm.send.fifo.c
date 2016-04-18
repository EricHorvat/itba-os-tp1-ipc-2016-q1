#include "./headers/comm.send.api.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <time.h>

#include <pthread.h>

#include "../../utils.h"

#include "headers/comm.fifo.h"

typedef struct {
	comm_callback_t cb;
	comm_addr_t* addr;
} comm_thread_info_t;

static void *data_listener(void *);

static void *data_listener(void *data) {

	comm_error_t *err;
	char *fifo;
	size_t read_bytes = 0;
	int fd;
	char* buffer;

	buffer = (char*)malloc(2048);

	comm_thread_info_t *info = (comm_thread_info_t*)data;

	buffer[0] = 'c';
	err = NEW(comm_error_t);

	fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(info->addr->host)+sizeof(FIFO_RESPONSE_EXTENSION));

	if (!fifo) {
		err->code = 1;
		err->msg = "[thread] FIFO could not be allocated";
		info->cb(err, info->addr, NULL);

		pthread_exit(NULL);

		return nil;
	}

	printf("[thread] Writing FIFO\n");

	strcat(fifo, FIFO_PATH_PREFIX);
	strcat(fifo, info->addr->host);
	strcat(fifo, FIFO_RESPONSE_EXTENSION);

	if (access(fifo, F_OK) == -1) {

		err->code = 2;
		err->msg = "[thread] Response FIFO not found";

		info->cb(err, info->addr, NULL);

		pthread_exit(NULL);

		return nil;

	}

	if ( (fd = open(fifo, O_RDONLY)) == -1) {

		unlink(fifo);
		err->code = 2;
		err->msg = "[thread] FIFO could not be written";
		info->cb(err, info->addr, NULL);

		pthread_exit(NULL);

		return nil;

	}
	

	while (*(buffer+read_bytes) != '\0') {
		read(fd, (buffer+(int)read_bytes), 1);
		read_bytes++;
	}
	close(fd);

	unlink(fifo);

	err->code = 0;
	err->msg = "Operacion Exitosa";

	info->cb(err, info->addr, buffer);

	pthread_exit(NULL);

	return nil;

}

void comm_send_data(void *data, size_t size, comm_addr_t *addr, comm_error_t *error) {

}

pthread_mutex_t calculating = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t done = PTHREAD_COND_INITIALIZER;

static void *data_writer(void *data) {

	int oldtype;

        

	comm_error_t *err;
	comm_thread_info_t *thread_arg;
	comm_thread_info_t *info = (comm_thread_info_t*)data;

	int fd;
	char *fifo;
	char *res_fifo;
	size_t written = 0;

	int mkfiforet;

	pthread_t listen_thread;
	int iret;

	/* allow the thread to be killed at any time */
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);

	err = NEW(comm_error_t);
	thread_arg = NEW(comm_thread_info_t);

	fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(addr->host)+sizeof(FIFO_REQUEST_EXTENSION));
	res_fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(addr->host)+sizeof(FIFO_RESPONSE_EXTENSION));

	if (!fifo || !res_fifo) {
		err->code = 1;
		err->msg = "FIFO could not be allocated";
		return info->cb(err, addr, NULL);
	}


	printf("Writing FIFO\n");

	sprintf(fifo, "%s%s%s", FIFO_PATH_PREFIX, addr->host, FIFO_REQUEST_EXTENSION);
	sprintf(res_fifo, "%s%s%s", FIFO_PATH_PREFIX, addr->host, FIFO_RESPONSE_EXTENSION);

	if (access(fifo, F_OK) == -1) {
		// fifo does not exist

		if (mkfifo(fifo, FIFO_PERMS) == -1) {
			err->code = 2;
			err->msg = "FIFO could not be created";
			return cb(err, addr, NULL);
		}

	}

	printf("Opening FIFO: %s\n", fifo);
	errno = 0;
	if ( (fd = open(fifo, O_WRONLY|O_NONBLOCK)) == -1) {
		unlink(fifo);
		err->code = 2;
		err->msg = "FIFO could not be opened";
		printf("panic: %d\t\tmsg:%s\n", errno, strerror(errno));
		return cb(err, addr, NULL);
	}

	pthread_cond_signal(&done);
	/** Hasta aca **/

	printf("A punto de escribir\n");

	while (written < size) {
		write(fd, data+written, 1);
		written++;
	}
	close(fd);
	unlink(fifo);


	if (access(res_fifo, F_OK) == -1) {
		// fifo does not exist

		if (mkfifo(res_fifo, FIFO_PERMS) == -1) {
			err->code = 4;
			err->msg = "Response FIFO could not be created";
			return cb(err, addr, NULL);
		}

	}

}



void comm_send_data_async(void * data, size_t size, comm_addr_t *addr, comm_callback_t cb) {

	comm_error_t *err;
	comm_thread_info_t *thread_arg;

	struct timespec abs_time, max_wait;
	pthread_t writer_thread;
	int writer_thread_err;

	thread_arg->cb = cb;
	thread_arg->addr = addr;

	pthread_mutex_lock(&calculating);

	max_wait.tv_sec = 20; // 20 = TIMEOUT;

	clock_gettime(CLOCK_REALTIME, &abs_time);
	abs_time.tv_sec += max_wait.tv_sec;
	abs_time.tv_nsec += max_wait.tv_nsec;

	pthread_create(&writer_thread, NULL, data_writer, (void*)thread_arg);

	writer_thread_err = pthread_cond_timedwait(&done, &calculating, &abs_time);

	if (writer_thread_err == ETIMEDOUT) {
		// timed out
		err->code = 7;
		err->msg = "Timed Out";
		cb(err, addr, nil);
	}
	if (!writer_thread_err) {
		pthread_mutex_lock(&calculating);
	}

	int fd;
	char *fifo;
	char *res_fifo;
	size_t written = 0;

	int mkfiforet;

	pthread_t listen_thread;
	int iret;

	err = NEW(comm_error_t);
	thread_arg = NEW(comm_thread_info_t);

	fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(addr->host)+sizeof(FIFO_REQUEST_EXTENSION));
	res_fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(addr->host)+sizeof(FIFO_RESPONSE_EXTENSION));

	if (!fifo || !res_fifo) {
		err->code = 1;
		err->msg = "FIFO could not be allocated";
		return cb(err, addr, NULL);
	}


	printf("Writing FIFO\n");

	sprintf(fifo, "%s%s%s", FIFO_PATH_PREFIX, addr->host, FIFO_REQUEST_EXTENSION);
	sprintf(res_fifo, "%s%s%s", FIFO_PATH_PREFIX, addr->host, FIFO_RESPONSE_EXTENSION);

	if (access(fifo, F_OK) == -1) {
		// fifo does not exist

		if (mkfifo(fifo, FIFO_PERMS) == -1) {
			err->code = 2;
			err->msg = "FIFO could not be created";
			return cb(err, addr, NULL);
		}

	}

	sleep(20);

	printf("Opening FIFO: %s\n", fifo);
	errno = 0;
	if ( (fd = open(fifo, O_WRONLY|O_NONBLOCK)) == -1) {
		unlink(fifo);
		err->code = 2;
		err->msg = "FIFO could not be opened";
		printf("panic: %d\t\tmsg:%s\n", errno, strerror(errno));
		return cb(err, addr, NULL);
	}

	printf("A punto de escribir\n");

	while (written < size) {
		write(fd, data+written, 1);
		written++;
	}
	close(fd);
	unlink(fifo);


	if (access(res_fifo, F_OK) == -1) {
		// fifo does not exist

		if (mkfifo(res_fifo, FIFO_PERMS) == -1) {
			err->code = 4;
			err->msg = "Response FIFO could not be created";
			return cb(err, addr, NULL);
		}

	}

	thread_arg->cb = cb;
	thread_arg->addr = addr;

	if ( (iret = pthread_create(&listen_thread, NULL, data_listener, (void*)thread_arg)) ) {
		fprintf(stderr, "pthread create returned %d\n", iret);
	}

	// cb(err, addr, (char*)data);
}

