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

static void *data_listener(void *);

/**
 * 
 *  SEND API
 * 
 * 
 **/

 static void *data_listener(void *info) {

 	comm_callback_t cb = (comm_callback_t)info;

 	comm_error_t *err;

 	printf("inside data_listener\n");

 	err = NEW(comm_error_t);
	err->code = 0;
	err->msg = "Operacion Exitosa";

 	sleep(2);

 	cb(err, NULL, "Hello");

 	return nil;

 }

void comm_send_data(void *data, size_t size, comm_addr_t *addr, comm_error_t *error) {

}


void comm_send_data_async(void * data, size_t size, comm_addr_t *addr, comm_callback_t cb) {

	comm_error_t *err;

	int fd;
	char *fifo;

	pthread_t listen_thread;
	int iret;

	fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(data));

	printf("Writing FIFO\n");

	strcat(fifo, FIFO_PATH_PREFIX);
	strcat(fifo, addr->host);

	mkfifo(fifo, FIFO_PERMS);

	fd = open(fifo, O_WRONLY|O_NONBLOCK);
	write(fd, "Hola", sizeof("Hola"));
	close(fd);

	unlink(fifo);

	if ( (iret = pthread_create(&listen_thread, NULL, data_listener, (void*)cb)) ) {
		fprintf(stderr, "pthread create returned %d\n", iret);
	}

	err = NEW(comm_error_t);
	err->code = 0;
	err->msg = "Operacion Exitosa";

	// cb(err, addr, (char*)data);
}

