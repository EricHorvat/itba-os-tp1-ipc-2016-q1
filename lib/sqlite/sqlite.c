#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite.h>
#include <errno.h>
#include <unistd.h>

#include <utils.h>

static void write_one_by_one_in_fd(char* str, int write_fd, int size) {
	int written_bytes = 0;
	do {
		written_bytes += write(write_fd, str + written_bytes, 1);
	} while (written_bytes < size);
}

static int callback(void* write_p, int argc, char** argv, char** azColName) {
	int   write_pipe = *((int*)write_p);
	int   i, n;
	char* str = malloc(MAX_QUERY_RESPONSE_LENGTH * sizeof(char));
	str[0]    = '\0';
	for (i = 0; i < argc; i++) {
		if (strlen(str) == 0)
			n = sprintf(str, "%s", argv[i] ? argv[i] : NULL_STR);
		else
			n = sprintf(str, "%s %s", str, argv[i] ? argv[i] : NULL_STR);
	}
	n = sprintf(str, "%s\n", str, argv[i] ? argv[i] : NULL_STR);
	write_one_by_one_in_fd(str, write_pipe, n);
	return SQLITE_OK;
}

char* run_sqlite_query(sql_connection_t* conn, char* query_text) {

	int    len;
	int    written_bytes = 0;
	int    i             = 0;
	char** asn_vector;
	char* query_response;
	char* aux;
	int readd;

	if (!conn) {
		ERROR("sql_connection es null");
		errno = ERR_SQL_CONNECTION_IS_NULL;
		return null;
	}

	len = strlen(query_text);

	asn_vector = malloc(MAX_COLUMNS * sizeof(char*));

	do {
		written_bytes += write(conn->write_pipe, query_text + written_bytes, 1);
	} while (written_bytes < len);
	
	do {
		query_response = malloc(sizeof(char) * MAX_QUERY_RESPONSE_LENGTH);
		aux      = malloc(sizeof(char) * 2);
		aux[1]         = '\0';
		readd      = 0;
		strcpy(query_response, "");
		do {
			readd += read(conn->read_pipe, aux, 1);
			strcat(query_response, aux);
		} while (strcmp("\n", aux));
		query_response[readd - 1] = '\0';
		asn_vector[i]             = query_response;
		i++;
	} while (strcmp(query_response, END_STR));
	return asn_vector[0];
}

int run_insert_sqlite_query(sql_connection_t* conn, sqlite_insert_query_t* query) {

	char * aux;
	aux = run_sqlite_query(conn, insert_query_to_str(query));

	if(strcmp(aux,END_STR)!= 0)
		return ERR_RUN_SQL_ERROR;

	return SQL_OK;
}

char* run_select_sqlite_query(sql_connection_t* conn, sqlite_select_query_t* query) {

	char* a;
	a = select_query_to_str(query);
	return run_sqlite_query(conn, a);
}

int run_delete_sqlite_query(sql_connection_t* conn, sqlite_delete_query_t* query) {

	char * aux;
	aux = run_sqlite_query(conn, delete_query_to_str(query));

	if(strcmp(aux,END_STR)!= 0)
		return ERR_RUN_SQL_ERROR;
	return SQL_OK;
}

int run_update_sqlite_query(sql_connection_t* conn, sqlite_update_query_t* query) {

	char * aux;
	aux = update_query_to_str(query);
	aux = run_sqlite_query(conn, aux);
	if(strcmp(aux,END_STR)!= 0)
		return ERR_RUN_SQL_ERROR;
	return SQL_OK;
}

char* to_fields_string(char** fields) {
	char* aux = malloc(sizeof(char) * MAX_QUERY_LENGTH);
	int   i   = 0, n=0;
	while (fields[i] != NULL) {
		if (i == 0)
			n = sprintf(aux, "%s", fields[i]);
		else
			n = sprintf(aux, "%s,%s", aux, fields[i]);
		i++;
		aux[n]='\0';
	}
	return aux;
}

int open_sql_conn(sql_connection_t* conn) {

	int fd[2][2], child_pid;

	if ((pipe(fd[1]) < 0) || (pipe(fd[0]) < 0)) {
		errno = ERR_CANT_CREATE_PIPE;
		return -1;
	}

	if ((child_pid = fork()) < 0) {
		errno = ERR_CANT_CREATE_SQL_SERVER;
		return -1;
	} else if (child_pid == 0) {

		close(fd[0][0]);
		close(fd[1][1]);
		init_sqlite_server(fd[1][0], fd[0][1]);
	} else {
		close(fd[0][1]);
		close(fd[1][0]);
		conn->read_pipe  = fd[0][0];
		conn->write_pipe = fd[1][1];
	}

	return SQL_OK;
}

int close_sql_conn(sql_connection_t* conn) {
	write(conn->write_pipe, SQL_CONNECTION_CLOSE_STR, strlen(SQL_CONNECTION_CLOSE_STR) + 1);
	close(conn->write_pipe);
	close(conn->read_pipe);

	return SQL_OK;
}

int init_sqlite_server(int read_pipe, int write_pipe) {

	sqlite3* db;
	char* errMsg;
	int      run = 1;
	char* query_buffer;
	char* aux;
	int q = 0, n;

	if (sqlite3_open("./files.db", &db)) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(-1);
	}

	while (run) {
		query_buffer = malloc(sizeof(char) * MAX_QUERY_LENGTH);

		q   = 0;
		aux = (char*)malloc(2*sizeof(char));
		strcpy(query_buffer, "");
		aux[1] = ZERO;
		do {
			read(read_pipe, aux, 1);
			strcat(query_buffer, aux);
		} while (strcmp(";", aux) && q++ < 100);

		if (strcmp(query_buffer, SQL_CONNECTION_CLOSE_STR)) {

			printf("Running: %s\n", query_buffer);

			if ((n = sqlite3_exec(db, query_buffer, callback, &write_pipe, &errMsg)) != SQLITE_OK) {
				fprintf(stderr, "SQL error: %s  %d\n", errMsg, n);
				sqlite3_free(errMsg);
			}
			aux = malloc(sizeof(char)*strlen(END_STR)+2);
			sprintf(aux,"%s\n",END_STR);
			write_one_by_one_in_fd(aux, write_pipe, strlen(aux));
		} else {
			run = 0;
		}
	}
	sqlite3_close(db);
	exit(SQL_OK);
}