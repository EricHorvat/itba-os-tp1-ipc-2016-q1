#include <helpers/sql_helpers.h>
#include <server.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <stdlib.h>
#include <types.h>
#include <errno.h>

sql_connection_t* sql_connection = NULL;

void set_sql_connection(sql_connection_t* sql_conn) {
	sql_connection = sql_conn;
}

char* ask_for_file_to_db(char* file_alias, fs_user_t* user) { 

	sqlite_select_query_t* query;

	char*   ans;
	char* alias_str;
	char* path_str;

	query = NEW(sqlite_select_query_t);

	create_select_query(query);

	set_select_query_table(query, "files");
	set_select_query_atribute(query, "path");

	alias_str = malloc((1 + strlen(file_alias) + 2) * sizeof(char));
	path_str  = malloc(1 + (strlen(user->home) + 3) * sizeof(char));

	sprintf(alias_str, "\"%s\"", file_alias);
	sprintf(path_str, "\'%s%%\'", user->home);

	set_select_query_where(query, "alias", "=", alias_str);
	set_select_query_where(query, "path", "LIKE", path_str);


	if (!strcmp((ans = run_select_sqlite_query(sql_connection, query)), END_STR)) {
		errno = EMPTY_RESPONSE;
		return NULL;
	}

	return ans;
}

int insert_alias_in_db(char* file_alias, fs_user_t* user) {

	sqlite_insert_query_t* query;
	int                    ans;
	char* path_str;
	char* alias_str;

	query = NEW(sqlite_insert_query_t);

	create_insert_query(query);
	set_insert_query_table(query, "files");

	path_str = malloc((1 + strlen(user->home)+ 1 + strlen(file_alias) + 2) * sizeof(char));
	alias_str = malloc((strlen(file_alias) + 1) * sizeof(char));

	sprintf(path_str, "\"%s/%s\"", user->home, file_alias);

	sprintf(alias_str, "\"%s\"", file_alias);

	set_insert_query_value(query, "path", path_str);
	set_insert_query_value(query, "owner", "2");
	set_insert_query_value(query, "alias", alias_str);

	ans = run_insert_sqlite_query(sql_connection, query);

	return EXPECTED_RESPONSE;
}

int user_identification_in_db(char* username, char* password, fs_user_t* user) {

	char* usern;
	char* pass;
	sqlite_select_query_t* query;
	char** response_array;
	char* response;

	query = NEW(sqlite_select_query_t);

	create_select_query(query);

	set_select_query_table(query, "users");
	set_select_query_atribute(query, "home");
	set_select_query_atribute(query, "user_id");
	set_select_query_atribute(query, "is_admin");

	usern = malloc((2 + strlen(username) + 1) * sizeof(char));
	pass  = malloc((2 + strlen(password) + 1) * sizeof(char));
	

	sprintf(usern, "\"%s\"", username);
	sprintf(pass, "\"%s\"", password);

	set_select_query_where(query, "username", "=", usern);
	set_select_query_where(query, "password", "=", pass);

	if (!strcmp((response = run_select_sqlite_query(sql_connection, query)), END_STR)) {
		return EMPTY_RESPONSE;
	}
	
	response_array = split_arguments(response);
	user->name     = username;
	user->home     = response_array[0];
	user->id       = atoi(response_array[1]);
	user->is_admin = atoi(response_array[2]);

	return EXPECTED_RESPONSE;
}


bool user_in_db(char* username) {

	char* user;
	sqlite_select_query_t* query;
	char** response_array;
	char* response;

	query = NEW(sqlite_select_query_t);

	create_select_query(query);

	set_select_query_table(query, "users");
	set_select_query_atribute(query, "home");
	
	user = malloc((2 + strlen(username) + 1) * sizeof(char));
	
	sprintf(user, "\"%s\"", username);
	
	set_select_query_where(query, "username", "=", user);
	
	if (!strcmp((response = run_select_sqlite_query(sql_connection, query)), END_STR)) {
		return no;
	}

	return yes;
}

int new_user_in_db(user_t* user) {
	sqlite_insert_query_t* query = malloc(sizeof(sqlite_insert_query_t));
	int                    ans;
	char* name_str;
	char* pass_str;
	char* home_str;
	char* root_str;

	create_insert_query(query);
	set_insert_query_table(query, "users");

	name_str = malloc((1 + strlen(user->username) + 2) * sizeof(char));
	pass_str = malloc((1 + strlen(user->password) + 2) * sizeof(char));
	home_str = malloc((4 + strlen(user->username) + 2) * sizeof(char));
	root_str = malloc(2 * sizeof(char));

	sprintf(name_str, "\"%s\"", user->username);
	sprintf(pass_str, "\"%s\"", user->password);
	sprintf(home_str, "\"/fs/%s\"", user->username);
	sprintf(root_str, "\"%d\"", (user->admin > 0) ? 1 : 0);

	set_insert_query_value(query, "username", name_str);
	set_insert_query_value(query, "password", pass_str);
	set_insert_query_value(query, "home", home_str);
	set_insert_query_value(query, "is_admin", root_str);

	run_insert_sqlite_query(sql_connection, query);

	return EXPECTED_RESPONSE;
}

bool update_pass_in_db(fs_user_t * user, char* new_pass){
	
	sqlite_update_query_t* query = malloc(sizeof(sqlite_update_query_t));
	
	char* pass_str;
	char * id_str;
	printf("1\n");
	create_update_query(query);
	printf("2\n");
	set_update_query_table(query, "users");
	printf("3\n");
	
	pass_str = malloc((1 + strlen(new_pass) + 2) * sizeof(char));
	id_str = malloc((6) * sizeof(char));
	sprintf(pass_str, "\"%s\"", new_pass);
	sprintf(id_str, "%d", user->id);
	printf("4\n",new_pass,pass_str);

	printf("5\n");
	set_update_query_value(query, "password", pass_str);
	printf("6\n");
	set_update_query_where(query, "user_id","=",id_str);

	printf("7\n");
	run_update_sqlite_query(sql_connection, query);
	printf("8\n");

	return EXPECTED_RESPONSE;
}