#include <stdio.h>
#include <yaml.h>



int main(int argc, char const *argv[]) {
	
	// https://www.wpsoftware.net/andrew/pages/libyaml.html

	FILE *config_file;
	yaml_parser_t yaml_parser;
	yaml_token_t token;

	config_file = fopen("server/config/config.yaml", "r");
	
	if (!yaml_parser_initialize(&yaml_parser)) {
		fputs("Failed to initialize parser!\n", stderr);
		return 1;
	}
	if (config_file == NULL) {
		fputs("Failed to open file!\n", stderr);
		return 2;
	}
	

	yaml_parser_set_input_file(&yaml_parser, config_file);

	do {
		yaml_parser_scan(&yaml_parser, &token);

		switch (token.type) {
			case YAML_STREAM_START_TOKEN:
				puts("STREAM START");
				break;
			case YAML_STREAM_END_TOKEN:
				puts("STREAM END");
				break;
			case YAML_KEY_TOKEN:   printf("(Key token)   "); break;
			case YAML_VALUE_TOKEN: printf("(Value token) "); break;

			case YAML_BLOCK_SEQUENCE_START_TOKEN: puts("<b>Start Block (Sequence)</b>"); break;
			case YAML_BLOCK_ENTRY_TOKEN:          puts("<b>Start Block (Entry)</b>");    break;
			case YAML_BLOCK_END_TOKEN:            puts("<b>End block</b>");              break;

			case YAML_BLOCK_MAPPING_START_TOKEN:  puts("[Block mapping]");            break;
			case YAML_SCALAR_TOKEN:  printf("scalar %s \n", token.data.scalar.value); break;

			default:
				printf("Got token of type %d\n", token.type);
		}
		if(token.type != YAML_STREAM_END_TOKEN)
			yaml_token_delete(&token);
	} while(token.type != YAML_STREAM_END_TOKEN);

	yaml_parser_delete(&yaml_parser);
	fclose(config_file);

	return 0;
}