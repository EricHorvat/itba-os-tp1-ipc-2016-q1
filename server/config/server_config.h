#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#define DEFAULT_CONFIG_FILE "server/config/config.yaml"
#define READ_PERMS "r"

#define INCOMING_CONNECTIONS_FIFO "server_incoming_connections.fifo"

typedef struct {
	char *connection_queue;
	unsigned int port;
} server_config_t;

int load_configuration(char *filename, server_config_t *config);

#endif