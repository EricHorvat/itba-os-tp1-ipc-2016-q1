#include <sqlite.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int create_select_query(sqlite_select_query_t * query){
	
	query->table = malloc(sizeof(char)*MAX_NAME_LENGTH);
	query->atributes = malloc(sizeof(char*)*MAX_COLUMNS);
	for(int i = 0; i < MAX_COLUMNS; i++){
		query->atributes[i] = NULL;
	}
	query->where = malloc(sizeof(char)*WHERE_LENGTH);
	sprintf(query->where,"1=1");
	return NO_QUERY_ERROR;
}

int set_select_query_table(sqlite_select_query_t * query, char * table){
	if(query == NULL) {errno = NULL_QUERY; return -1;}
		
	sprintf(query->table,"%s",table);
	return NO_QUERY_ERROR;
}

int set_select_query_atribute(sqlite_select_query_t * query, char * atribute){
	if(query == NULL) {errno = NULL_QUERY; return -1;}

	int i = 0;
	while(query->atributes[i] != NULL && i++<MAX_COLUMNS);
	
	if(i!=MAX_COLUMNS){
		char * atr = malloc(strlen(atribute));
		sprintf(atr,"%s",atribute);
		query->atributes[i]=atr;
	}

	return NO_QUERY_ERROR;
}

int set_select_query_where(sqlite_select_query_t * query, char * column, char * op, char * raw_value){
	if(query == NULL) {errno = NULL_QUERY; return -1;}
	
	sprintf(query->where,"%s%s%s",column,op,raw_value);
	return NO_QUERY_ERROR;
}

char * select_query_to_str(sqlite_select_query_t * query){
	int size;
	if(query == NULL) {errno = NULL_QUERY; return NULL;}
	if(query->table == NULL) {errno = NO_TABLE; return NULL;}
	if(query->atributes[0] == NULL) query->atributes[0] = "*";

	char * query_str = malloc(sizeof(char)*MAX_QUERY_LENGTH);

	size = sprintf(query_str,"SELECT %s FROM %s WHERE %s;",to_fields_string(query->atributes), query->table, query->where);
	query_str[size]='\0';
	return query_str;
}