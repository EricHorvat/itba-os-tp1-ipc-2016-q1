#include <client_config.h>
#include <getopt.h>
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <stdlib.h>

void process_arguments(int argc, char** argv, client_args_t* client_args) {

	// http://www.gnu.org/software/libc/manual/html_falsede/Example-of-Getopt.html#Example-of-Getopt

	int index;
	int c;

	if (!client_args)
		client_args = NEW(client_args_t);

	client_args->client_name = NULL;
	client_args->server      = NULL;
	client_args->protocol    = NULL;

	opterr = 0;
	while ((c = getopt(argc, argv, "n:s:p:")) != -1) {
		switch (c) {
			case 'n':
				client_args->client_name = optarg;
				break;
			case 's':
				client_args->server = optarg;
				break;
			case 'p':

				if (strcmp(optarg, "fd") != 0 && strcmp(optarg, "socket") != 0) {
					fprintf(stderr, "%s is not an allowed protocol\n", optarg);
					abort();
				}

				client_args->protocol = optarg;
				break;
			case '?':
				if (optopt == 'n')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				if (optopt == 's')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				if (optopt == 'p')
					fprintf(stderr, "Option -%c requires an argument.\n", optopt);
				else if (isprint(optopt))
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				return;
			default:
				printf("Aborting\n");
				abort();
		}
	}

	for (index = optind; index < argc; index++)
		printf("non-option argument %s\n", argv[index]);

	return;
}
