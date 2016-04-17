#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include "sqlite.h"

static int callback(void * notUsed, int argc, char **argv, char **azColName){
	int i;
		for(i=0; i<argc; i++){
			printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		}
		printf("\n");
		return 0;
	}

int run_sqlite_query(connection * conn, char * query_text){
	char *errMsg;

	printf("Running: %s\n",query_text);
	int n;
	if((n = sqlite3_exec(conn->db, query_text, callback, 0, &errMsg))!=SQLITE_OK ){
		fprintf(stderr, "SQL error: %s  %d\n", errMsg,n);
		sqlite3_free(errMsg);
	}
	return 0;
}

char * to_fields_string(char** fields){
	/*ADDING VALUES*/
	char * aux = malloc(MAX_QUERY_LENGTH);
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

int open_conn(connection * conn){
	
	int n;
	if( (n = sqlite3_open("./pokeDB.db", &(conn->db))) ){
			fprintf(stderr, "Can't open database: %s\n%d\n", sqlite3_errmsg(conn->db),n);
			sqlite3_close(conn->db);
			return(1);
		}
	return 0;
}

int close_conn(connection * conn){
	sqlite3_close(conn->db); 
}