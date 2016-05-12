#include <stdio.h>
#include <server.h>
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
#include <sqlite.h>
#include <file_utils.h>
#include <mqueue.h>
#include <server_utils.h>

#include <helpers/responder.h>
#include <helpers/sql_helpers.h>

#define MIN_THREADS 10

typedef struct {
	connection_t *connection;
	char *input;
} client_request_t;

pthread_mutex_t lock;

sql_connection_t *sql_connection;

static void listen_connections(server_config_t *config);
static void* server_responder(void* data);


static void* server_responder(void* data) {

	client_request_t *req;
	comm_error_t *err;
	parse_result_t *result;
	pthread_t self_;
	int pid = (int)getpid();
	long int self;
	char * log_str = NEW_LOG_STR();

	err = NEW(comm_error_t);
	self_ = pthread_self();
	req = (client_request_t*)data;

	self = (long int)self_;
	
	result = parse_encoded((const char*)req->input);

	S_INFO(log_str,"worker %d::thread %ld::client sent: [%s]", pid, self, result->kind);

	pthread_mutex_lock(&lock);
	if (strcmp(result->kind, "int") == 0) {

		S_INFO(log_str,"worker %d::thread %ld::client says: %d", pid, self, result->data.i);
		send_int((result->data.i)*2, req->connection, err);
		S_INFO(log_str,"worker %d::thread %ld::client i say: %d", pid, self, (result->data.i)*2);

	} else if ( strcmp(result->kind, "double") == 0) {

		S_INFO(log_str,"worker %d::thread %ld::client says: %f", pid, self, result->data.d);
		send_double((result->data.d)*2, req->connection, err);
		S_INFO(log_str,"worker %d::thread %ld::client i say: %f", pid, self, (result->data.d)*2);

	} else if ( strcmp(result->kind, "string") == 0 ) {

		S_INFO(log_str,"worker %d::thread %ld::client says: %s", pid, self, result->data.str);
		send_string(result->data.str+2, req->connection, err);

	} else if ( strcmp(result->kind, "command.get") == 0 ) {

		S_INFO(log_str,"worker %d::thread %ld::client says: %s", pid, self, result->data.get_cmd->path);
		process_get_cmd(req->connection, result->data.get_cmd);
		
	} else if ( strcmp(result->kind, "command.post") == 0 ) {

		S_INFO(log_str,"worker %d::thread %ld::client says: %s", pid, self, result->data.get_cmd->path);
		process_post_cmd(req->connection, result->data.post_cmd);
		
	} else if ( strcmp(result->kind, "command.login") == 0 ) {

		S_INFO(log_str,"worker %d::thread %ld::client says: %s", pid, self, result->data.login_cmd->user->username);
		process_login_cmd(req->connection, result->data.login_cmd);
		
	} else if ( strcmp(result->kind, "command.logout") == 0 ) {

		S_INFO(log_str,"worker %d::thread %ld::client says: %s", pid, self, result->data.login_cmd->user->username);
		process_logout_cmd(req->connection, result->data.login_cmd);
	}
	pthread_mutex_unlock(&lock);


	if (err->code) {
		S_ERROR(log_str,"worker %d::thread %ld::error: %d\tmsg:%s", pid, self, err->code, err->msg);
	} else {
		S_INFO(log_str,"worker %d::thread %ld::data sent successfully: (%s)", pid, self, err->msg);
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

	char * log_str = NEW_LOG_STR();

	sql_connection = NEW(sql_connection_t);
	
	open_sql_conn(sql_connection);
	
	connection = NEW(connection_t);
	connection->server_addr = NEW(comm_addr_t);
	
	threads = (pthread_t**)malloc(MIN_THREADS*sizeof(pthread_t*));

	if (address_from_url("socket://google:3000", connection->server_addr) != 0) {
		S_ERROR(log_str,"Invalid Address");
		abort();
	}

	S_INFO(log_str,"master::listening on name: %s", connection->server_addr->host);

	comm_listen(connection, nil);

	while (1) {
		S_WARN(log_str,"master::waiting for connections");
		comm_accept(connection, nil);

		if (!connection) {
			S_ERROR(log_str,"connection is null");
			break;
		}
		S_INFO(log_str,"master::opened connection for %s", connection->client_addr->host);

		childPID = fork();

		if (childPID > 0) {
			// parent
			S_INFO(log_str, "worker %d::created for %s", childPID, connection->client_addr->host);
		} else {
			if (childPID == -1) {
				S_ERROR(log_str,"Could not fork");
				abort();
			}

			if (pthread_mutex_init(&lock, NULL) != 0) {
					S_ERROR(log_str,"mutex init failed");
					exit(3);
			}

			while (1) {
				// si no manda nada cuelga aca
				S_WARN(log_str,"worker %d::waiting for data from %s", getpid(), connection->client_addr->host);
				command = comm_receive_data(connection, nil);
				S_INFO(log_str,"worker %d::%s says %s", getpid(), connection->client_addr->host, command);


				threads[current_thread] = NEW(pthread_t);

				request = NEW(client_request_t);

				request->connection = connection;
				request->input = (char*)malloc(strlen(command)+1);
				strcpy(request->input, command);

				

				if ( (pthread_ret = pthread_create(threads[current_thread], NULL, server_responder, request)) ) {
					S_ERROR(log_str,"worker %d::pthread create returned %d", getpid(), pthread_ret);
				}

				current_thread++;

				free(command);
			}
			
			S_INFO(log_str,"worker %d::ending", getpid());

			exit(0);
		}
	}

}

int main(int argc, char **argv) {
	
	
	init_mq();

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

	return 0;
}