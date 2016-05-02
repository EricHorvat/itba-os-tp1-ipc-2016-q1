#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <errno.h>
#include <unistd.h>
#include <sqlite.h>

static int callback(void * write_p, int argc, char **argv, char **azColName){
	int write_pipe = *((int*)write_p);
	int i, written_bytes = 0;
	for(i=0; i<argc; i++){
		char * str = malloc(/*CAMBIAR*/2048*sizeof(char));
		int len; 
		written_bytes = 0;
		sprintf(str,"%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		len = strlen(str);
		do {
			written_bytes += write(write_pipe, str + written_bytes,1);
		} while (written_bytes < len);
	}
	written_bytes = 0;
	char * e = "END";/*END SELECT*/
	do {
		written_bytes += write(write_pipe, e + written_bytes,1);
	} while (written_bytes < strlen(e));
	return 0;
}

int run_sqlite_query(sql_connection_t * conn, char * query_text){

	printf ("%s\n",query_text);
	int len = strlen(query_text);
	int written_bytes = 0;
	int i = 0;
	char ** asn_vector = malloc(8/*CHANGE*/*sizeof(char*));
	do {
		written_bytes += write(conn->write_pipe, query_text + written_bytes,1);
	} while (written_bytes < len);
	char * query_response;
	do{
		query_response = malloc(sizeof(char)*2048/*MAX_QUERY_RESPONSE*/);
		char * aux = malloc(sizeof(char)*2);
		strcpy(query_response,"");	
		do {
			read(conn->read_pipe,aux,1);
			strcat(query_response,aux);
		} while (strcmp("\0",aux));
		asn_vector[i]=query_response;
		i++;
	} while (strcmp(query_response, "END"));
	return i;
}

int run_insert_sqlite_query(sql_connection_t * conn, sqlite_insert_query_t * query){

	run_sqlite_query(conn, insert_query_to_str(query));
	return 0;
}

int run_select_sqlite_query(sql_connection_t * conn, sqlite_select_query_t * query){

	run_sqlite_query(conn, select_query_to_str(query));
	return 0;
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
}

int init_sqlite_server(int read_pipe, int write_pipe){

	sqlite3 * db;
	int run = 1;
	if (sqlite3_open("/Users/martin/Documents/TPSO/TP1/files.db", &db)){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(-1);
	}

	while (run){
		char * query_buffer = malloc(sizeof(char)*MAX_QUERY_LENGTH);

		int q=0,readd;
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
		} else{
			run = 0;
		}

	}
	sqlite3_close(db);
	exit(0);

}