#include "./headers/comm.send.api.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "../../utils.h"

#define PATH_PREFIX "/tmp/"
#define FIFO_PERMS 0666

/**
 * 
 *  SEND API
 * 
 * 
 **/

void send_data(void *data, unsigned int size, comm_addr_t *addr, comm_error_t *error) {

}

void send_data_async(void * data, unsigned int size, comm_addr_t *addr, comm_callback_t cb) {

	comm_error_t *err;

	int fd;
	char *fifo = (char*)malloc(sizeof(PATH_PREFIX)+sizeof(data));

	strcat(fifo, PATH_PREFIX);
	strcat(fifo, addr->host);

	mkfifo(fifo, FIFO_PERMS);

	fd = open(fifo, O_WRONLY);
	write(fd, "Hola", sizeof("Hola"));
	close(fd);

	unlink(fifo);

	err = (comm_error_t*)NEW(comm_error_t);
	err->code = 0;
	err->msg = "Operacion Exitosa";

	sleep(20);

	cb(err, addr, (char*)data);
}

