#include <helpers/sql_helpers.h>
#include <server.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <stdlib.h>

char * ask_sql(char * file_alias) { // por favor cambiale el nombre

	sqlite_select_query_t * query = malloc(sizeof(sqlite_select_query_t));
	
	char * ans;

	create_select_query(query);
	
	set_select_query_table(query,"files");
	set_select_query_atribute(query,"path");

	char * where_str = malloc((10+strlen(file_alias)+1)*sizeof(char)); 

	sprintf(where_str,"\"%s\"",file_alias);

	set_select_query_where(query,"alias", "=", where_str);

	printf("-----hola----\n");

	ans = run_select_sqlite_query(sql_connection,query);

	return ans;

}

int insert_alias_in_sql(char * file_alias) {

	sqlite_insert_query_t * query = malloc(sizeof(sqlite_insert_query_t));
	char * ans;
	
	create_insert_query(query);
	set_insert_query_table(query,"files");

	char * path_str = malloc((13+strlen(file_alias)+1)*sizeof(char)); 
	char * alias_str = malloc((strlen(file_alias)+1)*sizeof(char)); 

	sprintf(path_str, "\"/fs/mgoffan/%s\"", file_alias);

	sprintf(alias_str, "\"%s\"", file_alias);

	set_insert_query_value(query, "path", path_str);
	set_insert_query_value(query, "owner", "2");
	set_insert_query_value(query, "alias", alias_str);

	ans = run_insert_sqlite_query(sql_connection, query);

	/*if(!strcmp(ans, "END"))
		return 1;*/
	return 0;

}

