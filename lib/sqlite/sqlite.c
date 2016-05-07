#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite.h>
#include <errno.h>
#include <unistd.h>

#include <utils.h>

/*TO MARTIN, esto no lo hace write_one_by_one ?*/
static void write_one_by_one_in_fd(char * str, int write_fd, int size){
	int written_bytes=0;
	do {
		written_bytes += write(write_fd, str + written_bytes,1);
	} while (written_bytes < size);
}

static int callback(void * write_p, int argc, char **argv, char **azColName){
	int write_pipe = *((int*)write_p);
	int i;
	for(i=0; i<argc; i++){
		int n;
		char * str = malloc(/*CAMBIAR*/2048*sizeof(char));
		n = sprintf(str,"%s\n", argv[i] ? argv[i] : "NULL");
		write_one_by_one_in_fd(str, write_pipe, n);
	}
	return 0;
}

char * run_sqlite_query(sql_connection_t * conn, char * query_text){

	int len;
	int written_bytes = 0;
	int i = 0;
	char ** asn_vector;

	if (!conn) {
		ERROR("sql_connection es null");
		return null;
	}

	len = strlen(query_text);
	
	asn_vector = malloc(8/*CHANGE*/*sizeof(char*));
	
	do {
		written_bytes += write(conn->write_pipe, query_text + written_bytes,1);
	} while (written_bytes < len);
	char * query_response;
	do{
		query_response = malloc(sizeof(char)*2048/*MAX_QUERY_RESPONSE*/);
		char * aux = malloc(sizeof(char)*2);
		aux[1]='\0';
		int readd = 0;
		strcpy(query_response,"");	
		do {
			readd+=read(conn->read_pipe,aux,1);
			strcat(query_response,aux);	
		} while (strcmp("\n",aux));
		query_response[readd-1]='\0';	
		asn_vector[i]=query_response;
		i++;
	} while (strcmp(query_response, "END"));
	return asn_vector[0];
}

int run_insert_sqlite_query(sql_connection_t * conn, sqlite_insert_query_t * query){
 
	run_sqlite_query(conn, insert_query_to_str(query));

	return 0;
}

char * run_select_sqlite_query(sql_connection_t * conn, sqlite_select_query_t * query){

	char * a;
	a = select_query_to_str(query);
	return run_sqlite_query(conn, a);
}


int run_delete_sqlite_query(sql_connection_t * conn, sqlite_delete_query_t * query){

	run_sqlite_query(conn, delete_query_to_str(query));
	return 0;
}


int run_update_sqlite_query(sql_connection_t * conn, sqlite_update_query_t * query){

	run_sqlite_query(conn, update_query_to_str(query));
	return 0;
}

char * to_fields_string(char** fields){
	/*ADDING VALUES*/
	char * aux = malloc(sizeof(char)*MAX_QUERY_LENGTH);
	int i = 0;
	while(fields[i] != NULL){
		if(i==0)
			sprintf(aux,"%s",fields[i]);
		else
			sprintf(aux,"%s,%s",aux,fields[i]);
		i++;
	}
	return aux;
}

int open_sql_conn(sql_connection_t * conn){
	
	int fd[2][2], child_pid;

	if( (pipe(fd[1]) < 0) || (pipe(fd[0]) < 0) ){
		errno = CANT_CREATE_PIPE;
		return -1;
	}

	if ((child_pid = fork()) < 0){
		errno = CANT_CREATE_SQL_SERVER;
		return -1;
	} else if(child_pid == 0){

		close(fd[0][0]);
		close(fd[1][1]);
		init_sqlite_server(fd[1][0],fd[0][1]);
	} else{
		close(fd[0][1]);
		close(fd[1][0]);
		conn->read_pipe = fd[0][0];
		conn->write_pipe = fd[1][1];
	}

	return 0;
}

int close_sql_conn(sql_connection_t * conn){
	write(conn->write_pipe, SQL_CONNECTION_CLOSE_STR, strlen(SQL_CONNECTION_CLOSE_STR)+1);
	close(conn->write_pipe);
	close(conn->read_pipe);

	return 0;
}

int init_sqlite_server(int read_pipe, int write_pipe){

	sqlite3 * db;
	int run = 1;
	if (sqlite3_open("./files.db", &db)){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(-1);
	}

	while (run){
		char * query_buffer = malloc(sizeof(char)*MAX_QUERY_LENGTH);

		int q=0;
		char * aux = malloc(sizeof(char)*2);
		strcpy(query_buffer,"");
		aux[1]='\0';
		do {
			read(read_pipe,aux,1);
			strcat(query_buffer,aux);
		} while (strcmp(";",aux) && q++ < 100);

		if (strcmp(query_buffer, SQL_CONNECTION_CLOSE_STR)){
			
			char *errMsg;

			printf("Running: %s\n",query_buffer);

			int n;

			if((n = sqlite3_exec(db, query_buffer, callback, &write_pipe, &errMsg))!=SQLITE_OK ){
				fprintf(stderr, "SQL error: %s  %d\n", errMsg,n);
				sqlite3_free(errMsg);
			}

			write_one_by_one_in_fd("END\n",write_pipe, 4);
		} else{
			run = 0;
		}

	}
	sqlite3_close(db);
	exit(0);

}