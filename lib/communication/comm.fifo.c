#include "headers/communication.h"

#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "headers/comm.fifo.h"

comm_error_code_t comm_open(connection_t *comm) {

	comm_error_t *err;

	int fd;
	char *fifo;
	char *write_ptr;

	if (!comm || !comm->addr || !comm->connection_file)
		return -1;

	fifo = (char*)malloc(sizeof(FIFO_PATH_PREFIX)+sizeof(comm->connection_file));

	write_ptr = comm->addr->host;

	if (!fifo)
		return -1;

	printf("Opening connection\n");

	strcat(fifo, FIFO_PATH_PREFIX);
	strcat(fifo, comm->connection_file);

	printf("Creating FIFO\n");
	if (mkfifo(fifo, FIFO_PERMS) != -1) {
		printf("Opening FIFO\n");
		if ( (fd = open(fifo, O_WRONLY|O_NONBLOCK)) != -1 ) {
		
			printf("Writing FIFO\n");
			printf("Writing ");
			while (*write_ptr != '\0') {
				putchar(*write_ptr);
				write(fd, write_ptr, 1);
				write_ptr++;
			}
			
			// write(fd, comm->addr->host, strlen(comm->addr->host));

			close(fd);

			unlink(fifo);
		} else {
			return -1;
		}
	} else {
		return -1;
	}

	return 0;
}
