#include "sqlite.h"
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	sql_connection * conn = malloc(sizeof(conn));
	sqlite_update_query_t * query = malloc(sizeof(sqlite_update_query_t));

	open_conn(conn);

	create_update_query(query);

	set_update_query_table(query,"Effectivity");

	set_update_query_value(query,"EFFECTIVITY","22222");

	set_update_query_value(query,"IDELEMENT1","5");

	set_update_query_value(query,"IDELEMENT2","4");

	//set_update_query_where(query,"EFFECTIVITY = 11111");

	run_update_sqlite_query(conn,query);

	close_conn(conn);
	return 0;
}