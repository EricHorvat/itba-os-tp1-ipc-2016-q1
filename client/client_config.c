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

	opterr = 0;
	while ((c = getopt(argc, argv, "n:")) != -1) {
		switch (c) {
			case 'n':
				client_args->client_name = optarg;
				break;
			case '?':
				if (optopt == 'n')
					ERROR("Option -%c requires an argument.", optopt);
				else if (isprint(optopt))
					ERROR("Unknown option `-%c'.", optopt);
				else
					ERROR("Unknown option character `\\x%x'.", optopt);
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
