#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#define DEFAULT_CONFIG_FILE "server/config/config.json"

typedef struct {
	char *server_name;
	unsigned int port;
} server_config_t;

int load_configuration(char *filename, server_config_t *config);
char* process_arguments(int argc, char **argv);

#endif