#include <sqlite.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int create_delete_query(sqlite_delete_query_t* query) {

	query->table = malloc(sizeof(char) * MAX_NAME_LENGTH);
	query->where = malloc(sizeof(char) * WHERE_LENGTH);
	sprintf(query->where, "1=1");
	return NO_QUERY_ERROR;
}

int set_delete_query_table(sqlite_delete_query_t* query, char* table) {
	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return -1;
	}

	sprintf(query->table, "%s", table);
	return NO_QUERY_ERROR;
}

int set_delete_query_where(sqlite_delete_query_t* query, char* where) {
	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return -1;
	}

	sprintf(query->where, "%s", where);
	return NO_QUERY_ERROR;
}

char* delete_query_to_str(sqlite_delete_query_t* query) {
	char* query_str;

	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return NULL;
	}

	query_str = malloc(sizeof(char) * MAX_QUERY_LENGTH);
	sprintf(query_str, "DELETE FROM %s WHERE %s;", query->table, (query->where != NULL ? query->where : "1=1"));

	return query_str;
}
