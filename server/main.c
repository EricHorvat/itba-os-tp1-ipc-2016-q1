#include <stdio.h>
#include <yaml.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <ctype.h>

#define DEFAULT_CONFIG_FILE "server/config/config.yaml"

#define READ_PERMS "r"

int main(int argc, char **argv) {
	
	// https://www.wpsoftware.net/andrew/pages/libyaml.html

	FILE *config_file;
	yaml_parser_t yaml_parser;
	yaml_token_t token;

	// http://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html#Example-of-Getopt

	char *config_file_opt = NULL;
	int index;
	int c;

	opterr = 0;
	while ((c = getopt (argc, argv, "c:")) != -1) {
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
				return 1;
			default:
				abort();
		}
	}

	for (index = optind; index < argc; index++)
		printf ("Non-option argument %s\n", argv[index]);

	if (config_file_opt)
		config_file = fopen(config_file_opt, READ_PERMS);
	else
		config_file = fopen(DEFAULT_CONFIG_FILE, READ_PERMS);

	if (!yaml_parser_initialize(&yaml_parser)) {
		fputs("Failed to initialize parser!\n", stderr);
		return 1;
	}
	if (config_file == NULL) {
		fputs("Failed to open file!\n", stderr);
		return 2;
	}
	
	yaml_parser_set_input_file(&yaml_parser, config_file);

	/* BEGIN new code */
  do {
    yaml_parser_scan(&yaml_parser, &token);
    switch(token.type)
    {
    /* Stream start/end */
    case YAML_STREAM_START_TOKEN: puts("STREAM START"); break;
    case YAML_STREAM_END_TOKEN:   puts("STREAM END");   break;
    /* Token types (read before actual token) */
    case YAML_KEY_TOKEN:   printf("(Key token)   "); break;
    case YAML_VALUE_TOKEN: printf("(Value token) "); break;
    /* Block delimeters */
    case YAML_BLOCK_SEQUENCE_START_TOKEN: puts("<b>Start Block (Sequence)</b>"); break;
    case YAML_BLOCK_ENTRY_TOKEN:          puts("<b>Start Block (Entry)</b>");    break;
    case YAML_BLOCK_END_TOKEN:            puts("<b>End block</b>");              break;
    /* Data */
    case YAML_BLOCK_MAPPING_START_TOKEN:  puts("[Block mapping]");            break;
    case YAML_SCALAR_TOKEN:printf("scalar %s \n", token.data.scalar.value); break;
    /* Others */
    default:
      printf("Got token of type %d\n", token.type);
    }
    if(token.type != YAML_STREAM_END_TOKEN)
      yaml_token_delete(&token);
  } while(token.type != YAML_STREAM_END_TOKEN);
  yaml_token_delete(&token);
  /* END new code */

	yaml_parser_delete(&yaml_parser);
	fclose(config_file);

	return 0;
}