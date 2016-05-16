#ifndef __SQL_HELPERS_H__
#define __SQL_HELPERS_H__

#include <sqlite.h>
#include <types.h>
#include <utils.h>

typedef struct {
	char* name;
	char* home;
	boolean  is_admin;
	int   id;
} fs_user_t;

void set_sql_connection(sql_connection_t* sql_conn);
char* ask_for_file_to_db(char* file_alias, fs_user_t* user);
int insert_alias_in_db(char* file_alias, fs_user_t* user);
int user_identification_in_db(char* username, char* password, fs_user_t* user);
int new_user_in_db(user_t* user);
boolean user_in_db(char* username);
boolean update_pass_in_db(fs_user_t* user, char* new_pass);

#define EXPECTED_RESPONSE 0

#define EMPTY_RESPONSE 7000

#endif  // __SQL_HELPERS_H__