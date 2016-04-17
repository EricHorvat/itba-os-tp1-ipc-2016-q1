#include "sqlite.h"
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	connection * conn = malloc(sizeof(conn));
	sqlite_insert_query_t * query = malloc(sizeof(sqlite_insert_query_t));

	open_conn(conn);

	create_insert_query(query);

	set_insert_query_table(query,"Effectivity");

	set_insert_query_value(query,"EFFECTIVITY","11111");

	set_insert_query_value(query,"IDELEMENT1","4");

	set_insert_query_value(query,"IDELEMENT2","5");

	run_sqlite_query(conn,make_insert_query(query));

	close_conn(conn);
	return 0;
}