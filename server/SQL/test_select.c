#include "sqlite.h"
#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

int main(int argc, char const *argv[])
{
	connection * conn = malloc(sizeof(conn));
	sqlite_select_query_t * query = malloc(sizeof(sqlite_select_query_t));

	open_conn(conn);

	create_select_query(query);

	set_select_query_table(query,"Pokemon");

	set_select_query_atribute(query,"NAME");

	set_select_query_where(query,"NUMBER = 150");

	run_sqlite_query(conn,select_query_to_str(query));

	close_conn(conn);
	return 0;
}