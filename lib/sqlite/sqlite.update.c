#include <sqlite.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int create_update_query(sqlite_update_query_t* query) {

	int i = 0;

	query->table     = (char*)malloc(MAX_NAME_LENGTH);
	query->atributes = (char**)malloc(sizeof(char*) * MAX_COLUMNS);
	query->values    = (char**)malloc(sizeof(char*) * MAX_COLUMNS);
	for (; i < MAX_COLUMNS; i++) {
		query->atributes[i] = NULL;
		query->values[i]    = NULL;
	}
	query->where = (char*)malloc(WHERE_LENGTH);
	sprintf(query->where, "1=1");
	return NO_QUERY_ERROR;
}

int set_update_query_table(sqlite_update_query_t* query, char* table) {
	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return -1;
	}
	sprintf(query->table, "%s", table);
	return NO_QUERY_ERROR;
}

int set_update_query_value(sqlite_update_query_t* query, char* atribute, char* value) {
	
	char* atr;
	char* val;
	int i = 0;

	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return -1;
	}

	
	while (query->atributes[i] != NULL && i++ < MAX_COLUMNS)
		;

	if (i != MAX_COLUMNS) {
		atr = malloc(strlen(atribute));
		sprintf(atr, "%s", atribute);
		val = malloc(strlen(value));
		sprintf(val, "%s", value);
		query->atributes[i] = atr;
		query->values[i]    = val;
	}

	return NO_QUERY_ERROR;
}

int set_update_query_where(sqlite_update_query_t* query, char* column, char* op, char* raw_value) {
	if (query == NULL) {
		errno = ERR_NULL_QUERY;
		return -1;
	}

	sprintf(query->where, "%s AND %s %s %s", query->where, column, op, raw_value);
	return NO_QUERY_ERROR;
}

char* update_query_to_str(sqlite_update_query_t* query) {
	
	int    i   = 0;
	char** aux;
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

	aux = (char**)malloc(sizeof(char*) * MAX_COLUMNS);
	
	while (query->atributes[i] != NULL && i <= MAX_COLUMNS) {

		aux[i] = malloc(sizeof(char) * (strlen(query->atributes[i]) + strlen(query->values[i]) + 1));
		sprintf(aux[i], "%s=%s", query->atributes[i], query->values[i]);
		i++;
	}

	query_str = malloc(sizeof(char) * MAX_QUERY_LENGTH);
	sprintf(query_str, "UPDATE %s SET %s WHERE %s;", query->table, to_fields_string(aux), query->where);

	return query_str;
}