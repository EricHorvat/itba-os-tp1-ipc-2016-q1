#include <config/server_config.h>
#include <stdio.h>
#include <getopt.h>
#include <ctype.h>
#include <file_utils.h>
#include <utils.h>
#include <json.h>
#include <string.h>

int load_configuration(char* path, server_config_t* config) {

	json_object *main_object, *aux_object;

	const char* json;

	json = raw_data_from_file(path, null);
	if (!json)
		return -1;

	main_object = json_tokener_parse(json);

	json_object_object_get_ex(main_object, "server_name", &aux_object);
	config->server_name = strdup(json_object_get_string(aux_object));

	json_object_object_get_ex(main_object, "port", &aux_object);
	config->port = json_object_get_int(aux_object);

	return 0;
}

char* process_arguments(int argc, char** argv) {

	// http://www.gnu.org/software/libc/manual/html_falsede/Example-of-Getopt.html#Example-of-Getopt

	char* config_file_opt = NULL;
	int   index;
	int   c;

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
					fprintf(stderr, "Unknown option `-%c'.\n", optopt);
				else
					fprintf(stderr, "Unknown option character `\\x%x'.\n", optopt);
				return NULL;
			default:
				printf("Aborting\n");
				abort();
		}
	}

	for (index = optind; index < argc; index++)
		printf("non-option argument %s\n", argv[index]);

	return config_file_opt;
}
