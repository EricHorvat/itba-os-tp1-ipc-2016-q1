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

typedef struct{
	sqlite3 * db;
} connection;

int run_sqlite_query(connection * conn, char* query_text);

char * to_fields_string(char** fields);

int open_conn(connection * conn);

int close_conn(connection * conn);

#endif