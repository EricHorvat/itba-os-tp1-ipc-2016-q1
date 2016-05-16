#ifndef __SERVER_CONFIG_H__
#define __SERVER_CONFIG_H__

#include <stdlib.h>

#define DEFAULT_CONFIG_FILE "server/config/config.json"

typedef struct {
	char*        server_name;
	unsigned int port;
	char*				 db_file;	
	size_t			 min_threads; 
	size_t			 max_threads;
} server_config_t;

int load_configuration(char* filename, server_config_t* config);
char* process_arguments(int argc, char** argv);

#endif