#ifndef __SQLITE_ABM__
#define __SQLITE_ABM__

#include "sqlite.delete.h"
#include "sqlite.insert.h"
#include "sqlite.select.h"
#include "sqlite.update.h"
#include <sqlite3.h>

#define MAX_QUERY_LENGTH 1024

#define MAX_NAME_LENGTH 16

#define MAX_COLUMNS 10

#define WHERE_LENGTH 894 /*1024-128*/

#define SQL_CONNECTION_CLOSE_STR "CLOSE CONNECTION;"

/*ERRORS*/
#define NO_QUERY_ERROR 0

#define CANT_CREATE_SQL_SERVER 1000

#define CANT_CREATE_PIPE 1001

#define NULL_QUERY 2001

#define NO_TABLE 2002

#define NO_ATRIBUTE_NOR_VALUE 2003
/*END ERRORS */

typedef struct
{
	int read_pipe;
	int write_pipe;
}sql_connection;

int init_sqlite_server(int read_pipe, int write_pipe);

int run_sqlite_query(sql_connection * conn, char* query_text);

int run_insert_sqlite_query(sql_connection * conn, sqlite_insert_query_t * query);

int run_select_sqlite_query(sql_connection * conn, sqlite_select_query_t * query);

int run_delete_sqlite_query(sql_connection * conn, sqlite_delete_query_t * query);

int run_update_sqlite_query(sql_connection * conn, sqlite_update_query_t * query);

char * to_fields_string(char** fields);

int open_conn(sql_connection * conn);

int close_conn(sql_connection * conn);

#endif