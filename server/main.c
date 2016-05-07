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
#include <sqlite.h>
#include <file_utils.h>

#define MIN_THREADS 10

typedef struct {
	connection_t *connection;
	char *input;
} client_request_t;

pthread_mutex_t lock;

sql_connection_t *sql_connection;

static char * ask_sql(char * file_alias);

static int insert_alias_in_sql(char * file_alias);

static void process_get_cmd(connection_t *conn, command_get_t *cmd) {

	long size;
	void * contents;
	comm_error_t *err;
	char * p;
	char * aux;

	if(!strcmp( ( aux = ask_sql(cmd->path) ), "END" ) ){
		/*ERROR*/
		ERROR("FILE IS NOT IN DB");

		err = NEW(comm_error_t);

		send_data("\0",	1, conn, err);
		return;
	}


	size = strlen(aux)+2;
	p = (char*)malloc(size);
	memset(p, ZERO, size);

	sprintf(p, ".%s", aux);

	contents = raw_data_from_file(p);

	LOG("contents: %s", contents);

	err = NEW(comm_error_t);

	send_data(strdup(contents), size, conn, err);

	free(contents);

	if (err->code) {
		ERROR("send failed with code %d", err->code);
	}
}

static void process_post_cmd(connection_t *conn, command_post_t *post) {

	comm_error_t *err;

	char * path = malloc(14+strlen(post->dest));
	
	insert_alias_in_sql(post->dest);
	

	printf("TEST1\n");
	LOG("FOUND");
	
	sprintf(path,"./fs/mgoffan/%s\0",post->dest);

	printf("TEST\n");

	/**/file_from_row_data(path,post->data,post->size);

	LOG("file OK");

	err = NEW(comm_error_t);

	send_data(0, post->size, conn, err);

	if (err->code) {
		ERROR("send failed with code %d", err->code);
	}
}

static void* server_responder(void* data) {

	client_request_t *req;
	comm_error_t *err;
	parse_result_t *result;
	pthread_t self_;
	int pid = (int)getpid();
	long int self;

	err = NEW(comm_error_t);
	self_ = pthread_self();
	req = (client_request_t*)data;

	self = (long int)self_;

	result = parse_encoded((const char*)req->input);

	INFO("worker %d::thread %ld::client sent: [%s]\n", pid, self, result->kind);

	pthread_mutex_lock(&lock);
	if (strcmp(result->kind, "int") == 0) {

		INFO("worker %d::thread %ld::client says: %d\n", pid, self, result->data.i);
		send_int((result->data.i)*2, req->connection, err);
		INFO("worker %d::thread %ld::client i say: %d\n", pid, self, (result->data.i)*2);

	} else if ( strcmp(result->kind, "double") == 0) {

		INFO("worker %d::thread %ld::client says: %f\n", pid, self, result->data.d);
		send_double((result->data.d)*2, req->connection, err);
		INFO("worker %d::thread %ld::client i say: %f\n", pid, self, (result->data.d)*2);

	} else if ( strcmp(result->kind, "string") == 0 ) {

		INFO("worker %d::thread %ld::client says: %s\n", pid, self, result->data.str);
		send_string(result->data.str+2, req->connection, err);

	} else if ( strcmp(result->kind, "command.get") == 0 ) {

		INFO("worker %d::thread %ld::client says: %s\n", pid, self, result->data.get_cmd->path);

		process_get_cmd(req->connection, result->data.get_cmd);

		
	}else if ( strcmp(result->kind, "command.post") == 0 ) {

		INFO("worker %d::thread %ld::client says: %s\n", pid, self, result->data.get_cmd->path);

		process_post_cmd(req->connection, result->data.post_cmd);

		
	}
	pthread_mutex_unlock(&lock);




	if (err->code) {
		ERROR("worker %d::thread %ld::error: %d\tmsg:%s\n", pid, self, err->code, err->msg);
	} else {
		SUCCESS("worker %d::thread %ld::data sent successfully: (%s)\n", pid, self, err->msg);
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

	sql_connection = NEW(sql_connection_t);
	
	open_sql_conn(sql_connection);
	
	connection = NEW(connection_t);
	connection->server_addr = NEW(comm_addr_t);
	
	threads = (pthread_t**)malloc(MIN_THREADS*sizeof(pthread_t*));

	if (address_from_url("socket://google:3000", connection->server_addr) != 0) {
		ERROR("Invalid Address");
		abort();
	}

	SUCCESS("master::listening on name: %s", connection->server_addr->host);

	comm_listen(connection, nil);

	while (1) {
		WARN("master::waiting for connections");
		comm_accept(connection, nil);

		if (!connection) {
			ERROR("connection is null");
			break;
		}
		SUCCESS("master::opened connection for %s", connection->client_addr->host);

		childPID = fork();

		if (childPID > 0) {
			// parent
			SUCCESS("worker %d::created for %s", childPID, connection->client_addr->host);
		} else {
			if (childPID == -1) {
				ERROR("Could not fork");
				abort();
			}

			if (pthread_mutex_init(&lock, NULL) != 0) {
			    ERROR("mutex init failed");
			    exit(3);
			}

			while (1) {
				// si no manda nada cuelga aca
				WARN("worker %d::waiting for data from %s", getpid(), connection->client_addr->host);
				command = comm_receive_data(connection, nil);
				INFO("worker %d::%s says %s", getpid(), connection->client_addr->host, command);


				threads[current_thread] = NEW(pthread_t);

				request = NEW(client_request_t);

				request->connection = connection;
				request->input = (char*)malloc(strlen(command)+1);
				strcpy(request->input, command);

				

				if ( (pthread_ret = pthread_create(threads[current_thread], NULL, server_responder, request)) ) {
					ERROR("worker %d::pthread create returned %d", getpid(), pthread_ret);
				}

				current_thread++;


				free(command);
			}

			

			// process data
			// report to msq
			// ask SQL
			// respond
			// 
			
			INFO("worker %d::ending", getpid());

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

	// create_insert_query(query);

	// set_insert_query_table(query, "example");
	// set_insert_query_value(query, "value", "19");
	// // set_insert_query_value(query, "text", "chau");

	// run_sqlite_query(sql_connection, insert_query_to_str(query));

	// close_sql_conn(sql_connection);

	listen_connections(config);

	// while (incoming_connections_loop() > 0);

	return 0;
}

static char * ask_sql(char * file_alias){

	sqlite_select_query_t * query = malloc(sizeof(sqlite_select_query_t));
	
	char * ans;

	create_select_query(query);
	
	set_select_query_table(query,"files");

	set_select_query_atribute(query,"path");

	char * where_str = malloc((10+strlen(file_alias)+1)*sizeof(char)); 

	sprintf(where_str,"\"%s\"",file_alias);

	set_select_query_where(query,"alias", "=", where_str);

	printf("-----hola----\n");

	ans = run_select_sqlite_query(sql_connection,query);

	return ans;

}

static int insert_alias_in_sql(char * file_alias){

	sqlite_insert_query_t * query = malloc(sizeof(sqlite_insert_query_t));
	char * ans;
	
	create_insert_query(query);
	
	set_insert_query_table(query,"files");

	char * path_str = malloc((13+strlen(file_alias)+1)*sizeof(char)); 
	
	char * alias_str = malloc((strlen(file_alias)+1)*sizeof(char)); 

	sprintf(path_str,"\"/fs/mgoffan/%s\"",file_alias);

	sprintf(alias_str,"\"%s\"",file_alias);

	set_insert_query_value(query,"path", path_str);

	set_insert_query_value(query,"owner", "2");

	set_insert_query_value(query,"alias", alias_str);

	ans = run_insert_sqlite_query(sql_connection,query);

	/*if(!strcmp(ans, "END"))
		return 1;*/
	return 0;

}