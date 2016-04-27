#ifndef __CLIENT_CONFIG_H__
#define __CLIENT_CONFIG_H__

typedef struct {
	char *client_name;
	char *server;
	char *protocol;
} client_args_t;

void process_arguments(int argc, char **argv, client_args_t *client_args);

#endif