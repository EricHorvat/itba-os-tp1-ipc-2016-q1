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

bool check_user_logged(fs_user_t * user, connection_t * conn, comm_error_t * err);

void process_get_cmd(connection_t *conn, command_get_t *cmd) {

	long size;
	void * contents;
	comm_error_t *err;
	char * p;
	char * aux;
	err = NEW(comm_error_t);

	if(!check_user_logged(user,conn,err)){return;}

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

	err->msg = "OK";
	err->code = 0;

	send_data(strdup(contents), size, conn, err);

	free(contents);

	if (err->code) {
		ERROR("send failed with code %d", err->code);
	}
}

void process_post_cmd(connection_t *conn, command_post_t *post) {

	comm_error_t *err;
	size_t path_length;
	err = NEW(comm_error_t);
			
	if(!check_user_logged(user,conn,err)){return;}

	char * path = malloc(14+strlen(post->dest)); // 14??
	char * response;
	
	insert_alias_in_db(post->dest,user);
	
	path_length = sprintf(path,".%s/%s",user->home,post->dest);
	path[path_length] = '\0';
	
	/**/file_from_row_data(path,post->data,post->size);

	response = "file OK";

	err->msg="OK";

	send_data(strdup(response), strlen(response), conn, err);

	if (err->code) {
		ERROR("send failed with code %d", err->code);
	}
}

void process_login_cmd(connection_t *conn, command_login_t *login) {

	comm_error_t *err;
	err = NEW(comm_error_t);
			
	if(user != NULL){
		ERROR("USER ALREADY LOGGED");
		err->msg = "Already logged";
		send_data("Already logged", 14, conn, err);
	}
	else{
		char * aux;
		char * response;
		user = malloc(sizeof(fs_user_t));
		if((aux = user_identification_in_db(login->user->username,login->user->password,user)) != 0){
			/*ERROR*/
			ERROR("USER DOES NOT EXIST");

			err->msg = "Login failed";

			send_data("Login failed", 12, conn, err);
			user = NULL;
			return;
		}
		response = "Login successfully";
		err->msg = "Login successfully";
		send_data(strdup(response), strlen(response), conn, err);
	}
	return;
}

void process_logout_cmd(connection_t *conn, command_logout_t *logout){

	comm_error_t *err;
	err = NEW(comm_error_t);
	if(!check_user_logged(user,conn,err)){
		return;
	}
	err->msg="OK";
	send_data("Logged out", 10, conn, err);
	user=NULL;
	return;
}

void process_new_user_cmd(connection_t * conn, command_new_user_t *new_user){

	comm_error_t *err;
	err = NEW(comm_error_t);
	if(!check_user_logged(user,conn,err)){
		return;
	}
	if(user->is_admin == no){
		ERROR("USER HAVE NOT PERMISION");
		err->msg = "HAVE NOT PERMISION";
		send_data("HAVE NOT PERMISION", 10, conn, err);
		return;
	}
	printf("%s\n",new_user->user->username);
	create_user_folder(new_user->user->username);
	new_user_in_db(new_user->user->username);
	printf("ddd%s\n",new_user->user->username);
	//insert in db
	//returnok
}

bool check_user_logged(fs_user_t * user, connection_t * conn, comm_error_t * err){
	if(user == NULL){
		ERROR("USER NOT LOGGED");
		err->msg = "NOT logged";
		send_data("NOT logged", 10, conn, err);
		return no;
	}
	return yes;
}

void create_user_folder(char * username){
	int child_pid = 0;
	printf("%s\n", username);
	if((child_pid = fork())==0){
		char ** args, ** envp;
		char * cmd, * path;
		cmd = "/bin/mkdir";
		args = malloc(sizeof(char*)*3);
		envp = malloc(sizeof(char*));
		path = malloc(sizeof(char)*(5+strlen(username)+1));
		sprintf(path,"./fs/%s",username);
		envp[0]=NULL;
		args[0]=cmd;
		args[1]=path;
		args[2]=NULL;
		printf("a %s\n", username);
		execve(strdup(cmd),args,envp);
	}else if( child_pid < 0){
		ERROR("CANT FORK");
		return;
	}
	waitpid(child_pid);
}