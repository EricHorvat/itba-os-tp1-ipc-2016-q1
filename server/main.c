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
#include <math.h>
// #include <mqueue.h>
//#include <server_utils.h>
#include <helpers/logging_helpers.h>
#include <helpers/responder.h>
#include <helpers/sql_helpers.h>
#include <helpers/monitor_helpers.h>

#define MIN_THREADS 10

typedef struct {
	connection_t* connection;
	char*         input;
} client_request_t;

pthread_mutex_t lock;

sql_connection_t* sql_connection;

static void listen_connections(server_config_t* config);
static void* server_responder(void* data);

static void* server_responder(void* data) {

	client_request_t* 	req;
	comm_error_t*     	err;
	parse_result_t*   	result;
	int               	pid;
	long int          	self;
	char*             	log_str;
	bool				closing = no;

	log_str = (char*)malloc(MAX_LOG_LENGTH);
	pid     = (int)getpid();
	err     = NEW(comm_error_t);
	self    = (long int)pthread_self();
	req     = (client_request_t*)data;

	result = parse_encoded((const char*)req->input);

	post_status(pid, self, STATUS_OK);

	LOG_INFO(log_str, "worker %d::thread %ld::client sent: [%s]", pid, self, result->kind);

	pthread_mutex_lock(&lock);
	if (strcmp(result->kind, "int") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %d", pid, self, result->data.i);
		send_int((result->data.i) * 2, req->connection, err);
		LOG_INFO(log_str, "worker %d::thread %ld::client i say: %d", pid, self, (result->data.i) * 2);

	} else if (strcmp(result->kind, "double") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %f", pid, self, result->data.d);
		send_double((result->data.d) * 2, req->connection, err);
		LOG_INFO(log_str, "worker %d::thread %ld::client i say: %f", pid, self, (result->data.d) * 2);

	} else if (strcmp(result->kind, "string") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %s", pid, self, result->data.str);
		send_string(result->data.str + 2, req->connection, err);

	} else if (strcmp(result->kind, "command.get") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %s", pid, self, result->data.get_cmd->path);
		process_get_cmd(req->connection, result->data.get_cmd, err);

	} else if (strcmp(result->kind, "command.post") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %s", pid, self, result->data.post_cmd->dest);
		process_post_cmd(req->connection, result->data.post_cmd, err);

	} else if (strcmp(result->kind, "command.login") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %s", pid, self, result->data.login_cmd->user->username);
		process_login_cmd(req->connection, result->data.login_cmd, err);

	} else if (strcmp(result->kind, "command.logout") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %s", pid, self, result->kind);
		process_logout_cmd(req->connection, err);
	} else if (strcmp(result->kind, "command.close") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %s", pid, self, result->kind);
		req->connection->state = CONNECTION_STATE_CLOSED;
		closing = yes;
	} else if (strcmp(result->kind, "command.new_user") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %s", pid, self, result->data.new_user_cmd->user->username);
		process_new_user_cmd(req->connection, result->data.new_user_cmd, err);
	} else if (strcmp(result->kind, "command.change_pass") == 0) {

		LOG_INFO(log_str, "worker %d::thread %ld::client says: %s", pid, self, "PASSWORD");
		process_change_pass_cmd(req->connection, result->data.change_pass_cmd, err);
	}
	pthread_mutex_unlock(&lock);

	if (err->code) {
		post_status(pid, self, STATUS_ERROR);
		LOG_ERROR(log_str, "worker %d::thread %ld::error: %d\tmsg:%s", pid, self, err->code, err->msg);
	} else if (!closing){
		LOG_INFO(log_str, "worker %d::thread %ld::data sent successfully: (%s)", pid, self, err->msg);
	}

	post_status(pid, self, STATUS_DOWN);

	return nil;
}

static void listen_connections(server_config_t* config) {

	connection_t* connection;
	pid_t         childPID;
	char*         command;

	pthread_t**       threads;
	size_t            current_thread = 0;
	int               pthread_ret;
	client_request_t* request;

	char*  addr;
	size_t addr_len = 0;
	char*  log_str;

	comm_error_t *error;

	size_t num_threads = MIN_THREADS;

	log_str = (char*)malloc(MAX_LOG_LENGTH);

	sql_connection = NEW(sql_connection_t);

	open_sql_conn(sql_connection);

	connection              = NEW(connection_t);
	connection->server_addr = NEW(comm_addr_t);

#ifdef __FIFO__
	addr = malloc((addr_len = 7 + strlen(config->server_name) + 1));
	memset(addr, ZERO, addr_len);
	sprintf(addr, "fifo://%s", config->server_name);
	if (address_from_url(addr, connection->server_addr) != 0) {
		LOG_ERROR(log_str, "Invalid Address");
		post_status(0, 0, STATUS_ERROR);
		abort();
	}
#else
	addr = malloc((addr_len = 7 + strlen(config->server_name) + floorf(log10(config->port)) + 1 + 1));
	memset(addr, ZERO, addr_len);
	sprintf(addr, "fifo://%s:%d", config->server_name, config->port);
	if (address_from_url(addr, connection->server_addr) != 0) {
		LOG_ERROR(log_str, "Invalid Address");
		post_status(0, 0, STATUS_ERROR);
		abort();
	}
#endif
	threads = (pthread_t**)malloc(num_threads * sizeof(pthread_t*));

	LOG_INFO(log_str, "master::listening on name: %s", connection->server_addr->host);

	error = NEW(comm_error_t);

	comm_listen(connection, error);

	if (error->code) {
		ERROR("master::listen returned error %d", error->code);
		post_status(0, 0, STATUS_ERROR);
		abort();
	}

	LOG_SUCCESS(log_str, "master::listen ok");

	while (1) {
		LOG_WARN(log_str, "master::waiting for connections");
		post_status(0, 0, STATUS_IDLE);
		comm_accept(connection, error);

		if (error->code) {
			ERROR("master::accept returned error %d", error->code);
			post_status(0, 0, STATUS_ERROR);
			break;
		}

		if (!connection) {
			post_status(0, 0, STATUS_ERROR);
			LOG_ERROR(log_str, "connection is null");
			break;
		}
		post_status(0, 0, STATUS_OK);
		LOG_INFO(log_str, "master::opened connection for %s", connection->client_addr->host);

		childPID = fork();

		if (childPID > 0) {
			// parent
			LOG_INFO(log_str, "worker %d::created for %s", childPID, connection->client_addr->host);
		} else {
			if (childPID == -1) {
				LOG_ERROR(log_str, "Could not fork");
				abort();
			}

			post_status(getpid(), 0, STATUS_OK);
			if (pthread_mutex_init(&lock, NULL) != 0) {
				LOG_ERROR(log_str, "mutex init failed");
				post_status((int)getpid(), 0, STATUS_ERROR);
				exit(3);
			}

			while (!isConnectionClosed(connection)) {
				// si no manda nada cuelga aca
				LOG_WARN(log_str, "worker %d::waiting for data from %s", getpid(), connection->client_addr->host);
				post_status(getpid(), 0, STATUS_IDLE);
				command = comm_receive_data(connection, error);
				if (error->code) {
					post_status(getpid(), 0, STATUS_ERROR);
					ERROR("worker %d::received failed with code", getpid(), error->code);
					post_status(getpid(), 0, STATUS_WARN);
					exit(1);
				}
				post_status(getpid(), 0, STATUS_OK);
				LOG_INFO(log_str, "worker %d::%s says %s", getpid(), connection->client_addr->host, command);

				if (current_thread == num_threads - 1) {
					num_threads *= 2;
					threads = realloc(threads, num_threads * sizeof(pthread_t*));
				}

				threads[current_thread] = NEW(pthread_t);

				request = NEW(client_request_t);

				request->connection = connection;
				request->input      = (char*)malloc(strlen(command) + 1);
				strcpy(request->input, command);

				if ((pthread_ret = pthread_create(threads[current_thread], NULL, server_responder, request))) {
					post_status(getpid(), 0, STATUS_WARN);
					LOG_ERROR(log_str, "worker %d::pthread create returned %d", getpid(), pthread_ret);
				}

				current_thread++;

				free(command);
			}
			connection_close(connection);

			//if(shutdown) {sendshoutdown()}

			LOG_INFO(log_str, "worker %d::ending", getpid());
			post_status(getpid(), 0, STATUS_DOWN);
			exit(0);
		}
	}
}

int main(int argc, char** argv) {

	
	server_config_t* config;
	char*            config_file_opt;

	config = NEW(server_config_t);

#ifdef __LOGGING__
	init_mq();
#endif

	init_monitor();

	config_file_opt = process_arguments(argc, argv);

	if (config_file_opt) {
		if (load_configuration(config_file_opt, config)) {
			ERROR("Configuration file %s could not be opened", config_file_opt);
			abort();
		}
	} else {
		if (load_configuration(DEFAULT_CONFIG_FILE, config)) {
			ERROR("Configuration could not be opened");
			abort();
		}
	}

	post_status(0, 0, STATUS_OK);

	SUCCESS("Server started successfully with the following configuration:");
	LOG("Server Name:\t%s\n", config->server_name);
#ifdef __SOCKET__
	LOG("Port:\t%d\n", config->port);
#endif

	listen_connections(config);

	return 0;
}