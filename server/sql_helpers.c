#include <helpers/sql_helpers.h>
#include <server.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <stdlib.h>

sql_connection_t * sql_connection = NULL;

void set_sql_connection(sql_connection_t * sql_conn){
	sql_connection = sql_conn;
}


char * ask_for_file_to_db(char * file_alias, fs_user_t * user) { // por favor cambiale el nombre

	sqlite_select_query_t * query = malloc(sizeof(sqlite_select_query_t));
	
	char * ans;

	create_select_query(query);
	
	set_select_query_table(query,"files");
	set_select_query_atribute(query,"path");

	char * alias_str = malloc((10+strlen(file_alias)+1)*sizeof(char)); 
	char * path_str = malloc(10+(strlen(user->home)+1)*sizeof(char)); 

	sprintf(alias_str,"\"%s\"",file_alias);
	sprintf(path_str,"\'%s%%\'",user->home);
	
	set_select_query_where(query,"alias", "=", alias_str);
	set_select_query_where(query,"path", "LIKE", path_str);

	ans = run_select_sqlite_query(sql_connection,query);

	return ans;

}

int insert_alias_in_db(char * file_alias, fs_user_t * user) {

	sqlite_insert_query_t * query = malloc(sizeof(sqlite_insert_query_t));
	char * ans;
	
	create_insert_query(query);
	set_insert_query_table(query,"files");

	char * path_str = malloc((13+strlen(file_alias)+1)*sizeof(char)); 
	char * alias_str = malloc((strlen(file_alias)+1)*sizeof(char)); 

	sprintf(path_str, "\"%s%s\"", user->home, file_alias);

	sprintf(alias_str, "\"%s\"", file_alias);

	set_insert_query_value(query, "path", path_str);
	set_insert_query_value(query, "owner", "2");
	set_insert_query_value(query, "alias", alias_str);

	ans = run_insert_sqlite_query(sql_connection, query);

	/*if(!strcmp(ans, "END"))
		return 1;*/
	return 0;

}


/*RETURN USER HOME*/
int user_identification_in_db(char * username, char * password, fs_user_t * user) {

	sqlite_select_query_t * query = malloc(sizeof(sqlite_select_query_t));
	
	create_select_query(query);
	
	set_select_query_table(query,"users");
	set_select_query_atribute(query,"home");
	set_select_query_atribute(query,"user_id");
	set_select_query_atribute(query,"is_admin");

	char * usern = malloc((2+strlen(username)+1)*sizeof(char)); 
	char * pass = malloc((2+strlen(password)+1)*sizeof(char)); 
	char * admin = malloc(2*sizeof(char)); 
	char * response;
	char * id_str;

	sprintf(usern,"\"%s\"",username);
	sprintf(pass,"\"%s\"",password);

	set_select_query_where(query,"username", "=", usern);
	set_select_query_where(query,"password", "=", pass);

	if(!strcmp( (response = run_select_sqlite_query(sql_connection,query)), "END" ) ){
		/*ERROR*/
		return -1;
	}
	char ** response_array;
	response_array = split_arguments(response);
	user->name = username;
	user->home = response_array[0];
	user->id = atoi(response_array[1]);
	user->is_admin = atoi(response_array[2]);

	return 0;
}

