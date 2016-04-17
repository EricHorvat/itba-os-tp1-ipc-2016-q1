#ifndef __SQLITE_INSERT__
#define __SQLITE_INSERT__

typedef struct{
	char * table;
	char ** atributes;
	char ** values;
}sqlite_insert_query_t;

int create_insert_query(sqlite_insert_query_t * query);

int set_insert_query_table(sqlite_insert_query_t * query, char * table);

int set_insert_query_value(sqlite_insert_query_t * query, char * atribute, char * value);

char * make_insert_query(sqlite_insert_query_t * query);
#endif