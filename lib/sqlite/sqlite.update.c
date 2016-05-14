#include <sqlite.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int create_update_query(sqlite_update_query_t* query) {

	query->table     = malloc(sizeof(char) * MAX_NAME_LENGTH);
	query->atributes = malloc(sizeof(char*) * MAX_COLUMNS);
	query->values    = malloc(sizeof(char*) * MAX_COLUMNS);
	for (int i = 0; i < MAX_COLUMNS; i++) {
		query->atributes[i] = NULL;
		query->values[i]    = NULL;
	}
	query->where = malloc(sizeof(char) * WHERE_LENGTH);
	sprintf(query->where, "1=1");
	return NO_QUERY_ERROR;
}

int set_update_query_table(sqlite_update_query_t* query, char* table) {
	if (query == NULL) {
		errno = NULL_QUERY;
		return -1;
	}
	sprintf(query->table, "%s", table);
	return NO_QUERY_ERROR;
}

int set_update_query_value(sqlite_update_query_t* query, char* atribute, char* value) {
	if (query == NULL) {
		errno = NULL_QUERY;
		return -1;
	}

	int i = 0;
	while (query->atributes[i] != NULL && i++ < MAX_COLUMNS)
		;

	if (i != MAX_COLUMNS) {
		char* atr = malloc(strlen(atribute));
		sprintf(atr, "%s", atribute);
		char* val = malloc(strlen(value));
		sprintf(val, "%s", value);
		query->atributes[i] = atr;
		query->values[i]    = val;
	}

	return NO_QUERY_ERROR;
}

int set_update_query_where(sqlite_update_query_t* query, char* where) {
	if (query == NULL) {
		errno = NULL_QUERY;
		return -1;
	}

	sprintf(query->where, "%s", where);
	return NO_QUERY_ERROR;
}

char* update_query_to_str(sqlite_update_query_t* query) {
	if (query == NULL) {
		errno = NULL_QUERY;
		return NULL;
	}
	if (query->table == NULL) {
		errno = NO_TABLE;
		return NULL;
	}
	if (query->atributes[0] == NULL) {
		errno = NO_ATRIBUTE_NOR_VALUE;
		return NULL;
	}

	char** aux = malloc(sizeof(char*) * MAX_COLUMNS);
	int    i   = 0;
	while (query->atributes[i] != NULL && i <= MAX_COLUMNS) {

		aux[i] = malloc(sizeof(char) * (strlen(query->atributes[i]) + strlen(query->values[i]) + 1));
		sprintf(aux[i], "%s=%s", query->atributes[i], query->values[i]);
		i++;
	}

	char* query_str = malloc(sizeof(char) * MAX_QUERY_LENGTH);
	sprintf(query_str, "UPDATE %s SET %s WHERE %s;", query->table, to_fields_string(aux), query->where);

	return query_str;
}