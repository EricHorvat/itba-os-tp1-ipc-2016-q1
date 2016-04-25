#include "sqlite.h"
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	sql_connection * conn = malloc(sizeof(conn));
	sqlite_delete_query_t * query = malloc(sizeof(sqlite_delete_query_t));

	open_conn(conn);

	create_delete_query(query);

	set_delete_query_table(query,"Effectivity");

	//set_delete_query_where(query,"EFFECTIVITY = 22222");

	run_delete_sqlite_query(conn,query);

	close_conn(conn);
	return 0;
}