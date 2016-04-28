#include <stdio.h>

#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <communication.h>
#include <config/server_config.h>
#include <utils.h>
#include <serialization.h>

#define MIN_THREADS 10

typedef struct {
	connection_t *connection;
	char *input;
} client_request_t;

pthread_mutex_t lock;

static void* server_responder(void* data) {

	client_request_t *req;
	comm_error_t *err;
	parse_result_t *result;
	pthread_t self;
	int pid = (int)getpid();

	err = NEW(comm_error_t);
	self = pthread_self();
	req = (client_request_t*)data;

	result = parse_encoded((const char*)req->input);

	printf(ANSI_COLOR_BLUE"worker %d::thread %ld::client sent: [%s]\n"ANSI_COLOR_RESET, pid, self, result->kind);

	pthread_mutex_lock(&lock);
	if (strcmp(result->kind, "int") == 0) {
		printf(ANSI_COLOR_CYAN"worker %d::thread %ld::client says: %d\n"ANSI_COLOR_RESET, pid, self, result->data.i);
		send_int((result->data.i)*2, req->connection, COMMUNICATION_SERVER_CLIENT, err);
		printf(ANSI_COLOR_MAGENTA"worker %d::thread %ld::client i say: %d\n"ANSI_COLOR_RESET, pid, self, (result->data.i)*2);
	} else if ( strcmp(result->kind, "double") == 0) {
		printf(ANSI_COLOR_CYAN"worker %d::thread %ld::client says: %f\n"ANSI_COLOR_RESET, pid, self, result->data.d);
		send_double((result->data.d)*2, req->connection, COMMUNICATION_SERVER_CLIENT, err);
		printf(ANSI_COLOR_MAGENTA"worker %d::thread %ld::client i say: %f\n"ANSI_COLOR_RESET, pid, self, (result->data.d)*2);
	} else if ( strcmp(result->kind, "string") == 0 ) {
		printf(ANSI_COLOR_CYAN"worker %d::thread %ld::client says: %s\n"ANSI_COLOR_RESET, pid, self, result->data.str);
		send_string(result->data.str+2, req->connection, COMMUNICATION_SERVER_CLIENT, err);
	}
	pthread_mutex_unlock(&lock);

	if (err->code) {
		fprintf(stderr, ANSI_COLOR_RED "worker %d::thread %ld::error: %d\tmsg:%s\n" ANSI_COLOR_RESET, pid, self, err->code, err->msg);
	} else {
		printf(ANSI_COLOR_GREEN"worker %d::thread %ld::data sent successfully: (%s)\n"ANSI_COLOR_RESET, pid, self, err->msg);
	}

	return nil;

}

static void listen_connections(server_config_t *config) {

	connection_t *connection;
	pid_t childPID;
	char *command;
	parse_result_t *result;

	pthread_t **threads;
	size_t current_thread = 0;
	int pthread_ret;
	client_request_t *request;

	
	connection = NEW(connection_t);
	connection->server_addr = NEW(comm_addr_t);
	
	threads = (pthread_t**)malloc(MIN_THREADS*sizeof(pthread_t*));

	if (address_from_url("fd://google", connection->server_addr) != 0) {
		fprintf(stderr, ANSI_COLOR_RED "Invalid Address\n" ANSI_COLOR_RESET);
		abort();
	}

	printf(ANSI_COLOR_GREEN"master::listening on name: %s\n"ANSI_COLOR_RESET, connection->server_addr->host);

	comm_listen(connection, nil);

	while (1) {
		printf(ANSI_COLOR_YELLOW"master::waiting for connections\n"ANSI_COLOR_RESET);
		comm_accept(connection, nil);

		if (!connection) {
			fprintf(stderr, ANSI_COLOR_RED"connection is null\n"ANSI_COLOR_RESET);
			break;
		}
		printf(ANSI_COLOR_GREEN"master::opened connection for %s\n"ANSI_COLOR_RESET, connection->client_addr->host);

		childPID = fork();

		if (childPID > 0) {
			// parent
			printf(ANSI_COLOR_GREEN"worker %d::created for %s\n"ANSI_COLOR_RESET, childPID, connection->client_addr->host);
		} else {
			if (childPID == -1) {
				fprintf(stderr, ANSI_COLOR_RED "Could not fork\n" ANSI_COLOR_RESET);
				abort();
			}

			if (pthread_mutex_init(&lock, NULL) != 0) {
			    printf("\n mutex init failed\n");
			    exit(3);
			}

			while (1) {
				// si no manda nada cuelga aca
				printf(ANSI_COLOR_YELLOW"worker %d::waiting for data from %s\n"ANSI_COLOR_RESET, getpid(), connection->client_addr->host);
				command = comm_receive_data(connection, COMMUNICATION_CLIENT_SERVER, nil);
				printf(ANSI_COLOR_CYAN"worker %d::%s says %s\n"ANSI_COLOR_RESET, getpid(), connection->client_addr->host, command);


				threads[current_thread] = NEW(pthread_t);

				request = NEW(client_request_t);

				request->connection = connection;
				request->input = (char*)malloc(strlen(command)+1);
				strcpy(request->input, command);

				

				if ( (pthread_ret = pthread_create(threads[current_thread], NULL, server_responder, request)) ) {
					fprintf(stderr, ANSI_COLOR_RED"worker %d::pthread create returned %d\n"ANSI_COLOR_RED, getpid(), pthread_ret);
				}

				current_thread++;


				free(command);
			}

			

			// process data
			// report to msq
			// ask SQL
			// respond
			// 
			
			printf("worker %d::ending\n", getpid());

			exit(0);
		}
	}

}


int main(int argc, char **argv) {
	
	
	server_config_t *config;
	char *config_file_opt;
	

	config = NEW(server_config_t);
	
	config_file_opt = process_arguments(argc, argv);

	if (config_file_opt)
		load_configuration(config_file_opt, config);
	else
		load_configuration(DEFAULT_CONFIG_FILE, config);

	printf("connection~>%s\tport~>%d\n", config->connection_queue, config->port);

	listen_connections(config);

	// while (incoming_connections_loop() > 0);

	return 0;
}