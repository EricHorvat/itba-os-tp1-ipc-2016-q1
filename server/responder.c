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

void process_get_cmd(connection_t *conn, command_get_t *cmd) {

	long size;
	void * contents;
	comm_error_t *err;
	char * p;
	char * aux;

	if(!strcmp( ( aux = ask_sql(cmd->path) ), "END" ) ){
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
	int g=0;

	printf("%d\n", g++);

	char * path = malloc(14+strlen(post->dest)); // 14??
	
	printf("%d\n", g++);

	insert_alias_in_sql(post->dest);
	

	printf("TEST1\n");
	LOG("FOUND");
	
	path_length = sprintf(path,"./fs/mgoffan/%s",post->dest);
	path[path_length] = '\0';

	printf("TEST\n");

	/**/file_from_row_data(path,decode_to_raw_data(post->data),post->size);

	LOG("file OK");

	err = NEW(comm_error_t);

	send_data(0, post->size, conn, err);

	if (err->code) {
		ERROR("send failed with code %d", err->code);
	}
}
