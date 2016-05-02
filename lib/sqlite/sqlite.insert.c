#include <sqlite.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int create_insert_query(sqlite_insert_query_t * query){
	
	query->table = malloc(sizeof(char)*MAX_NAME_LENGTH);
	query->atributes = malloc(sizeof(char*)*MAX_COLUMNS);
	query->values = malloc(sizeof(char*)*MAX_COLUMNS);
	for(int i = 0; i < MAX_COLUMNS; i++){
		query->atributes[i] = NULL;
		query->values[i] = NULL;
	}
	
	return NO_QUERY_ERROR;
}

int set_insert_query_table(sqlite_insert_query_t * query, char * table){
	if(query == NULL) {errno = NULL_QUERY; return -1;}
		
	sprintf(query->table,"%s",table);	
	return NO_QUERY_ERROR;
}

int set_insert_query_value(sqlite_insert_query_t * query, char * atribute, char * value){
	if(query == NULL) {errno = NULL_QUERY; return -1;}
	
	int i = 0;
	while(query->atributes[i] != NULL && i++<MAX_COLUMNS);
	
	if(i!=MAX_COLUMNS){
		char * atr = malloc(strlen(atribute));
		sprintf(atr,"%s",atribute);
		char * val = malloc(strlen(value));
		sprintf(val,"%s",value);
		query->atributes[i]=atr;
		query->values[i]=val;
	}

	return NO_QUERY_ERROR;
}

char * insert_query_to_str(sqlite_insert_query_t * query){
	if(query == NULL) {errno = NULL_QUERY; return NULL;}
	if(query->table == NULL) {errno = NO_TABLE; return NULL;}
	if(query->atributes[0] == NULL) {errno = NO_ATRIBUTE_NOR_VALUE; return NULL;}

	char * query_str = malloc(sizeof(char)*MAX_QUERY_LENGTH);	
	sprintf(query_str,"INSERT INTO %s(%s) VALUES(%s);",query->table, to_fields_string(query->atributes), to_fields_string(query->values));
	return query_str;
}

