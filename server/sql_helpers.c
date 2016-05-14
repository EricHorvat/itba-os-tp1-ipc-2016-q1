#include <helpers/sql_helpers.h>
#include <server.h>
#include <string.h>
#include <stdio.h>
#include <utils.h>
#include <stdlib.h>
#include <types.h>

sql_connection_t* sql_connection = NULL;

void set_sql_connection(sql_connection_t* sql_conn) {
	sql_connection = sql_conn;
}

char* ask_for_file_to_db(char* file_alias, fs_user_t* user) {  // por favor cambiale el nombre

	sqlite_select_query_t* query;

	char*   ans;
	char* alias_str;
	char* path_str;

	query = NEW(sqlite_select_query_t);

	create_select_query(query);

	set_select_query_table(query, "files");
	set_select_query_atribute(query, "path");

	alias_str = malloc((10 + strlen(file_alias) + 1) * sizeof(char));
	path_str  = malloc(10 + (strlen(user->home) + 1) * sizeof(char));

	sprintf(alias_str, "\"%s\"", file_alias);
	sprintf(path_str, "\'%s%%\'", user->home);

	set_select_query_where(query, "alias", "=", alias_str);
	set_select_query_where(query, "path", "LIKE", path_str);

	ans = run_select_sqlite_query(sql_connection, query);

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

	path_str = malloc((13 + strlen(file_alias) + 1) * sizeof(char));
	alias_str = malloc((strlen(file_alias) + 1) * sizeof(char));

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

	if (!strcmp((response = run_select_sqlite_query(sql_connection, query)), "END")) {
		/*ERROR*/
		return -1;
	}
	
	response_array = split_arguments(response);
	user->name     = username;
	user->home     = response_array[0];
	user->id       = atoi(response_array[1]);
	user->is_admin = atoi(response_array[2]);

	return 0;
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

	ans = run_insert_sqlite_query(sql_connection, query);

	/*if(!strcmp(ans, "END"))
		return 1;*/
	return 0;
}
