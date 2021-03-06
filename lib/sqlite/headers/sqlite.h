#ifndef __SQLITE_ABM__
#define __SQLITE_ABM__

#include <sqlite3.h>
#include <sqlite.delete.h>
#include <sqlite.insert.h>
#include <sqlite.select.h>
#include <sqlite.update.h>

#define MAX_QUERY_LENGTH 1024
#define MAX_NAME_LENGTH 16
#define MAX_COLUMNS 10
#define WHERE_LENGTH 894 /*1024-128*/

#define SQL_CONNECTION_CLOSE_STR "CLOSE CONNECTION;"
#define END_STR "END"
#define NULL_STR "NULL"
#define MAX_QUERY_RESPONSE_LENGTH 2048

/*ERRORS*/
#define SQL_OK 0

#define ERR_RUN_SQL_ERROR 999

#define NO_QUERY_ERROR 0

#define ERR_CANT_CREATE_SQL_SERVER 1000
#define ERR_CANT_CREATE_PIPE 1001
#define ERR_SQL_CONNECTION_IS_NULL 1002

#define ERR_NULL_QUERY 2001
#define ERR_NO_TABLE 2002
#define ERR_NO_ATRIBUTE_NOR_VALUE 2003
/*END ERRORS */

typedef struct {
	int   read_pipe;
	int   write_pipe;
	char* db_file;
} sql_connection_t;

int init_sqlite_server(char*, int read_pipe, int write_pipe);
char* run_sqlite_query(sql_connection_t* conn, char* query_text);
int run_insert_sqlite_query(sql_connection_t* conn, sqlite_insert_query_t* query);
char* run_select_sqlite_query(sql_connection_t* conn, sqlite_select_query_t* query);
int run_delete_sqlite_query(sql_connection_t* conn, sqlite_delete_query_t* query);
int run_update_sqlite_query(sql_connection_t* conn, sqlite_update_query_t* query);
char* to_fields_string(char** fields);
int open_sql_conn(sql_connection_t* conn);
int close_sql_conn(sql_connection_t* conn);

#endif
