#include <helpers/responder.h>
#include <serialization.h>
#include <server.h>
#include <string.h>
#include <utils.h>
#include <file_utils.h>
#include <stdio.h>
#include <helpers/sql_helpers.h>
#include <sqlite.h>
#include <stdlib.h>

fs_user_t * user = NULL;

void process_get_cmd(connection_t *conn, command_get_t *cmd) {

	long size;
	void * contents;
	comm_error_t *err;
	char * p;
	char * aux;

	if(!strcmp( ( aux = ask_for_file_to_db(cmd->path, user) ), "END" ) ){
		/*ERROR*/
		ERROR("FILE IS NOT IN DB");

		err = NEW(comm_error_t);

		send_data("\0",	1, conn, err);
		return;
	}


	size = strlen(aux)+2;
	p = (char*)malloc(size);
	memset(p, ZERO, size);

	sprintf(p, ".%s", aux);

	contents = raw_data_from_file(p, &size);

	LOG("contents: %s", contents);

	err = NEW(comm_error_t);

	send_data(strdup(contents), size, conn, err);

	free(contents);

	if (err->code) {
		ERROR("send failed with code %d", err->code);
	}
}

void process_post_cmd(connection_t *conn, command_post_t *post) {

	comm_error_t *err;
	size_t path_length;
	
	char * path = malloc(14+strlen(post->dest)); // 14??
	char * response;
	
	insert_alias_in_db(post->dest,user);
	

	LOG("FOUND");
	
	path_length = sprintf(path,".%s/%s",user->home,post->dest);
	path[path_length] = '\0';
	
	/**/file_from_row_data(path,post->data,post->size);

	response = "file OK";
	LOG("file OK");

	err = NEW(comm_error_t);

	send_data(strdup(response), strlen(response), conn, err);

	if (err->code) {
		ERROR("send failed with code %d", err->code);
	}
}

void process_login_cmd(connection_t *conn, command_login_t *login) {

	comm_error_t *err;
	if(user != NULL){
		ERROR("USER ALREADY LOGGED");
		err = NEW(comm_error_t);
		send_data("Already logged", 14, conn, err);
	}
	else{
		char * aux;
		char * response;
		user = malloc(sizeof(fs_user_t));
		if((aux = user_identification_in_db(login->user->username,login->user->password,user)) != 0){
			/*ERROR*/
			ERROR("USER DOES NOT EXIST");

			err = NEW(comm_error_t);

			send_data("Login failed", 12, conn, err);
			user = NULL;
			return;
		}
		response = "Login successfully";
		send_data(strdup(response), strlen(response), conn, err);
	}
	return;
}

void process_logout_cmd(connection_t *conn, command_logout_t *logout){

	comm_error_t *err;
	if(user != NULL){
		err = NEW(comm_error_t);
		send_data("Logged out", 10, conn, err);
	}
	else{
		ERROR("ALREADY NOT LOGGED");
		err = NEW(comm_error_t);
		send_data("Already not logged", 18, conn, err);
	}
	return;
}