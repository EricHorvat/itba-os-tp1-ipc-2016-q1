#include "./headers/comm.send.api.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include <pthread.h>

#include "../../utils.h"

#include "headers/comm.fifo.h"

typedef struct {
	comm_callback_t cb;
	comm_addr_t* addr;
} comm_thread_info_t;

static void *data_listener(void *);

/**
 * 
 *  SEND API
 * 
 * 
 **/

 static void *data_listener(void *data) {

 	comm_error_t *err;
 	char *fifo;
 	size_t read_bytes = 0;
 	int fd;
 	char* buffer = (char*)malloc(2048);

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

	if (mkfifo(fifo, FIFO_PERMS) != -1) {

		if ( (fd = open(fifo, O_RDONLY)) != -1) {

			while (*(buffer+read_bytes) != '\0') {
				read(fd, (buffer+(int)read_bytes), 1);
				read_bytes++;
			}
			close(fd);

		} else {
			unlink(fifo);
			err->code = 2;
			err->msg = "FIFO could not be written";
			info->cb(err, info->addr, NULL);

			pthread_exit(NULL);

			return nil;
		}

		unlink(fifo);

		err->code = 0;
		err->msg = "Operacion Exitosa";

		info->cb(err, info->addr, buffer);

		pthread_exit(NULL);

		return nil;

	} else {
		err->code = 2;
		err->msg = "FIFO could not be created";
		info->cb(err, info->addr, NULL);

		pthread_exit(NULL);

		return nil;
	}

 	pthread_exit(NULL);

 	return nil;

 }

void comm_send_data(void *data, size_t size, comm_addr_t *addr, comm_error_t *error) {

}


void comm_send_data_async(void * data, size_t size, comm_addr_t *addr, comm_callback_t cb) {

	comm_error_t *err;
	comm_thread_info_t *thread_arg;

	int fd;
	char *fifo;
	size_t written = 0;

	pthread_t listen_thread;
	int iret;

	err = NEW(comm_error_t);
	thread_arg = NEW(comm_thread_info_t);

	fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(addr->host)+sizeof(FIFO_REQUEST_EXTENSION));

	if (!fifo) {
		err->code = 1;
		err->msg = "FIFO could not be allocated";
		return cb(err, addr, NULL);
	}


	printf("Writing FIFO\n");

	strcat(fifo, FIFO_PATH_PREFIX);
	strcat(fifo, addr->host);
	strcat(fifo, FIFO_REQUEST_EXTENSION);

	if (mkfifo(fifo, FIFO_PERMS) != -1) {

		if ( (fd = open(fifo, O_WRONLY|O_NONBLOCK)) != -1) {

			while (written < size) {
				write(fd, data+written, 1);
				written++;
			}
			close(fd);

		} else {
			unlink(fifo);
			err->code = 2;
			err->msg = "FIFO could not be written";
			return cb(err, addr, NULL);
		}

		unlink(fifo);

		thread_arg->cb = cb;
		thread_arg->addr = addr;

		if ( (iret = pthread_create(&listen_thread, NULL, data_listener, (void*)thread_arg)) ) {
			fprintf(stderr, "pthread create returned %d\n", iret);
		}
	} else {
		err->code = 2;
		err->msg = "FIFO could not be created";
		return cb(err, addr, NULL);
	}

	// cb(err, addr, (char*)data);
}

