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

/*ERRORS*/
#define NO_QUERY_ERROR 0

#define NULL_QUERY 1001

#define NO_TABLE 1002

#define NO_ATRIBUTE_NOR_VALUE 1003
/*END ERRORS */


typedef struct{
	sqlite3 * db;
} connection;

int run_sqlite_query(connection * conn, char* query_text);

char * to_fields_string(char** fields);

int open_conn(connection * conn);

int close_conn(connection * conn);

#endif