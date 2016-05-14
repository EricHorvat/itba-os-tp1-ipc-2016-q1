#ifndef __SQLITE_DELETE__
#define __SQLITE_DELETE__

typedef struct {
	char* table;
	char* where;
} sqlite_delete_query_t;

int create_delete_query(sqlite_delete_query_t* query);

int set_delete_query_table(sqlite_delete_query_t* query, char* table);

int set_delete_query_where(sqlite_delete_query_t* query, char* where);

char* delete_query_to_str(sqlite_delete_query_t* query);
#endif