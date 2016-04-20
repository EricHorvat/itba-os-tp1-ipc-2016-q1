#include <config/server_config.h>
#include <yaml.h>
#include <stdio.h>

#include <utils.h>

int load_configuration(char *filename, server_config_t *config) {

	// https://www.wpsoftware.net/andrew/pages/libyaml.html

	FILE *config_file;
	yaml_parser_t yaml_parser;
	yaml_token_t token;

	bool will_read_key = false, will_read_value = false;
	char *last_key = (char*)malloc(1);
	char *last_value = (char*)malloc(1);


	config_file = fopen(filename, READ_PERMS);

	if (config_file == NULL) {
		fputs("Failed to open file!\n", stderr);
		return 2;
	}
	if (!yaml_parser_initialize(&yaml_parser)) {
		fputs("Failed to initialize parser!\n", stderr);
		return 1;
	}

	
	yaml_parser_set_input_file(&yaml_parser, config_file);

	/* BEGIN new code */
	do {

		yaml_parser_scan(&yaml_parser, &token);

		switch(token.type) {
    	/* Stream start/end */
			case YAML_STREAM_START_TOKEN: // puts("STREAM START");
				break;
			case YAML_STREAM_END_TOKEN: // puts("STREAM END");
				break;
    	/* Token types (read before actual token) */
			case YAML_KEY_TOKEN:
				will_read_key = true;
				will_read_value = false;
				break;
			case YAML_VALUE_TOKEN:
				will_read_value = true;
				will_read_key = false;
				break;
    	/* Block delimeters */
			case YAML_BLOCK_SEQUENCE_START_TOKEN:// puts("<b>Start Block (Sequence)</b>");
				break;
			case YAML_BLOCK_ENTRY_TOKEN:// puts("<b>Start Block (Entry)</b>");
				break;
			case YAML_BLOCK_END_TOKEN:// puts("<b>End block</b>");
				break;
    	/* Data */
			case YAML_BLOCK_MAPPING_START_TOKEN:
				will_read_key = false;
				will_read_value = false;
				break;
			case YAML_SCALAR_TOKEN:
				if (will_read_key) {
					last_key = (char*)malloc(strlen((char*)token.data.scalar.value)+1);
					strcpy(last_key, (char*)token.data.scalar.value);
					will_read_key = false;
				}
				else if (will_read_value) {
					last_value = (char*)malloc(strlen((char*)token.data.scalar.value)+1);
					strcpy(last_value, (char*)token.data.scalar.value);
					will_read_value = false;
				}
				break;
    	/* Others */
			default:
				printf("Got token of type %d\n", token.type);
		}

		if (	token.type == YAML_SCALAR_TOKEN
			&&	last_value[0] != ZERO
			&&	last_key[0] != ZERO ) {

			if (strcmp(last_key, "fifo") == 0) {
				config->connection_queue = (char*)malloc(strlen(last_value)+1);
				strcpy(config->connection_queue, last_value);
			}
			else if (strcmp(last_key, "port") == 0) {
				config->port = atoi(last_value);
			}
			else {
				fprintf(stderr, "Unknown key %s\n", last_key);
			}
			last_value[0] = '\0';
			last_key[0] = '\0';
		}

		if (token.type != YAML_STREAM_END_TOKEN)
			yaml_token_delete(&token);


	} while(token.type != YAML_STREAM_END_TOKEN);
	yaml_token_delete(&token);
  /* END new code */

	yaml_parser_delete(&yaml_parser);
	fclose(config_file);

	return 0;
}