#ifndef __SQLITE_DELETE__
#define __SQLITE_DELETE__

typedef struct{
	char * table;
	char ** atributes;
	char ** values;
	char * where;
	char * query
}sqlite_update_query_t;

int create_update_query(sqlite_update_query_t * query);

int set_update_query_table(sqlite_update_query_t * query, char * table);

int set_update_query_value(sqlite_update_query_t * query, char * atribute, char * value);

int set_update_query_where(sqlite_update_query_t * query);

int run_update_query(sqlite_update_query_t * query);

#endif