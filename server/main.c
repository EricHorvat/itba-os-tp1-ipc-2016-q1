#include <stdio.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <config/server_config.h>

#include <utils.h>

static char* process_arguments(int argc, char **argv);
static void incoming_connections_loop();

static char* process_arguments(int argc, char **argv) {

	// http://www.gnu.org/software/libc/manual/html_falsede/Example-of-Getopt.html#Example-of-Getopt

	char *config_file_opt = NULL;
	int index;
	int c;

	opterr = 0;
	while ((c = getopt(argc, argv, "c:")) != -1) {
		switch (c) {
			case 'c':
				config_file_opt = optarg;
				break;
			case '?':
				if (optopt == 'c')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt))
					fprintf(stderr, "Unkfalsewn option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unkfalsewn option character `\\x%x'.\n", optopt);
				return NULL;
			default:
				printf("Aborting\n");
				abort();
		}
	}

	for (index = optind; index < argc; index++)
		printf ("falsen-option argument %s\n", argv[index]);

	return config_file_opt;
}

static void incoming_connections_loop() {

	char *fifo = "/tmp/google.req";
	char *buffer = (char*)malloc(2048);
	size_t read_bytes = 0;
	int fd;
	
	if (access(fifo, F_OK) == -1) {

		if (mkfifo(fifo, 0666) == -1) {
			
			return 3;
		}

	}

	if ( (fd = open(fifo, O_RDONLY)) == -1) {

		printf("[thread] FIFO could not be written\n");

		unlink(fifo);

		return 3;

	}
	

	while (1/**(buffer+read_bytes) != '\0'*/) {
		read(fd, (buffer+(int)read_bytes), 1);
		putchar( *(buffer+read_bytes) );
		read_bytes++;
	}
	close(fd);

	unlink(fifo);

	printf("buffer: (%s)\n", buffer);


	while (1) {

		/**
		 *
		 *   Pseudo-code
		 *
		 *   read until \n
		 *   get who it is
		 *   read request fifo
		 *   fork
		 *   listen to next one
		 * 
		 *
		 */

	}

	

}

int main(int argc, char **argv) {
	
	
	server_config_t *config;
	char *config_file_opt;
	

	config = NEW(server_config_t);
	
	config_file_opt = process_arguments(argc, argv);

	if (config_file_opt)
		load_configuration(config_file_opt, config);
	else
		load_configuration(DEFAULT_CONFIG_FILE, config);

	printf("connection~>%s\tport~>%d\n", config->connection_queue, config->port);

	incoming_connections_loop();

	return 0;
}