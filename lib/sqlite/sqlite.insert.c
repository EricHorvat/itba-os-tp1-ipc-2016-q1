#include <sqlite.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int create_insert_query(sqlite_insert_query_t* query) {

	int i = 0;

	query->table     = malloc(sizeof(char) * MAX_NAME_LENGTH);
	query->atributes = malloc(sizeof(char*) * MAX_COLUMNS);
	query->values    = malloc(sizeof(char*) * MAX_COLUMNS);
	for (; i < MAX_COLUMNS; i++) {
		query->atributes[i] = NULL;
		query->values[i]    = NULL;
	}

	return NO_QUERY_ERROR;
}

int set_insert_query_table(sqlite_insert_query_t* query, char* table) {
	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return -1;
	}

	sprintf(query->table, "%s", table);
	return NO_QUERY_ERROR;
}

int set_insert_query_value(sqlite_insert_query_t* query, char* atribute, char* value) {
	int i = 0;
	char* atr;
	char* val;

	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return -1;
	}
	
	while (query->atributes[i] != NULL && i++ < MAX_COLUMNS)
		;

	if (i != MAX_COLUMNS) {
		atr = (char*)malloc(strlen(atribute));
		sprintf(atr, "%s", atribute);
		val = (char*)malloc(strlen(value));
		sprintf(val, "%s", value);
		query->atributes[i] = atr;
		query->values[i]    = val;
	}

	return NO_QUERY_ERROR;
}

char* insert_query_to_str(sqlite_insert_query_t* query) {
	char* query_str;
	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return NULL;
	}
	if (query->table == NULL) {
		errno = ERR_NO_TABLE;
		return NULL;
	}
	if (query->atributes[0] == NULL) {
		errno = ERR_NO_ATRIBUTE_NOR_VALUE;
		return NULL;
	}

	query_str = malloc(sizeof(char) * MAX_QUERY_LENGTH);
	sprintf(query_str, "INSERT INTO %s(%s) VALUES(%s);", query->table, to_fields_string(query->atributes), to_fields_string(query->values));
	return query_str;
}
