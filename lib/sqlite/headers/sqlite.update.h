#ifndef __SQLITE_UPDATE__
#define __SQLITE_UPDATE__

typedef struct {
	char*  table;
	char** atributes;
	char** values;
	char*  where;
} sqlite_update_query_t;

int create_update_query(sqlite_update_query_t* query);

int set_update_query_table(sqlite_update_query_t* query, char* table);

int set_update_query_value(sqlite_update_query_t* query, char* atribute, char* value);

int set_update_query_where(sqlite_update_query_t* query, char* where);

char* update_query_to_str(sqlite_update_query_t* query);

#endif