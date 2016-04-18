#ifndef __SQLITE_SELECT__
#define __SQLITE_SELECT__

typedef struct{
	char * table;
	char ** atributes;
	char * where;
}sqlite_select_query_t;

int create_select_query(sqlite_select_query_t * query);

int set_select_query_table(sqlite_select_query_t * query, char * table);

int set_select_query_atribute(sqlite_select_query_t * query, char * atribute);

int set_select_query_where(sqlite_select_query_t * query, char * where);

char * select_query_to_str(sqlite_select_query_t * query);

#endif