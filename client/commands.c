#include <commands.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <stdlib.h>
#include <file_utils.h>

typedef struct {
	char *name;
	int (*cmd)(connection_t *, char *args);
	char *help;
} client_command_t;

static bool initialized_commands = no;
static bool logged = no;

static void initialize_commands();

static int cmd_open(connection_t *, char *args);
static int cmd_sendi(connection_t *conn, char* args);
static int cmd_sendd(connection_t *conn, char* args);
static int cmd_get(connection_t *conn, char* args);
static int cmd_post(connection_t *conn, char* args);
static int cmd_login(connection_t *conn, char* args);
static int cmd_logout(connection_t *conn, char* args);

static client_command_t **commands;

static void initialize_commands() {

	int i = 0;

	commands = (client_command_t**)malloc(50*sizeof(client_command_t*));

	commands[i] = NEW(client_command_t);
	commands[i]->name = "open";
	commands[i]->cmd = &cmd_open;
	commands[i++]->help = "Open help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "close";
	commands[i]->cmd = &cmd_open;
	commands[i++]->help = "Close help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "sendi";
	commands[i]->cmd = &cmd_sendi;
	commands[i++]->help = "Sendi help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "sendd";
	commands[i]->cmd = &cmd_sendd;
	commands[i++]->help = "Sendd help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "sends";
	commands[i]->cmd = &cmd_open;
	commands[i++]->help = "Sends help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "help";
	commands[i]->cmd = &cmd_open;
	commands[i++]->help = "Help help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "get";
	commands[i]->cmd = &cmd_get;
	commands[i++]->help = "Get help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "post";
	commands[i]->cmd = &cmd_post;
	commands[i++]->help = "Post help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "login";
	commands[i]->cmd = &cmd_login;
	commands[i++]->help = "Login help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "logout";
	commands[i]->cmd = &cmd_logout;
	commands[i++]->help = "Logout help";

	commands[i] = NULL;
}

int cmd_parse(connection_t *conn, char * cmd) {

	char *start = cmd;
	// char **cmds = commands;
	int i = 0, j = 0;

	if (!initialized_commands) {
		initialize_commands();
		initialized_commands = yes;
	}

	while (*cmd != '\n' && *cmd != '\0') {

		if (*cmd == ' ') {

			while (commands[j] != NULL) {

				if (strncmp(start, commands[j]->name, i) == 0) {
					int error_code;
					error_code = commands[j]->cmd(conn, cmd+1);
					return error_code;
				}

				j++;
			}
			printf(ANSI_COLOR_YELLOW"[%s] Command not found\n"ANSI_COLOR_RESET, start);
			return 1;
		}
		i++;
		cmd++;
	}

	return 1;


}

static int cmd_open(connection_t *conn, char *args) {

	int conn_error = -4;

	conn->server_addr = NEW(comm_addr_t);

	if (address_from_url(args, conn->server_addr)) {
		return 5;
	}

	if ( (conn_error = connection_open(conn) != 0) ) {
		fprintf(stderr, ANSI_COLOR_RED"error %d creating connection\n"ANSI_COLOR_RESET, conn_error);
		return conn_error;
	}

	return 0;

}

static int cmd_sendi(connection_t *conn, char* args) {

	comm_error_t *err;
	parse_result_t *presult;

	if (conn->state != CONNECTION_STATE_OPEN) {

		WARN("Please open connection first");

		return 1;
	}

	err = NEW(comm_error_t);

	INFO("sending %s", args);

	send_int(atoi(args), conn, err);

	INFO("sent %s", args);

	if (err->code) {
		ERROR("send failed err code: %d msg: %s", err->code, err->msg);
		return err->code;
	}

	INFO("fetching");
	presult = receive(conn, err);
	INFO("fetched");

	if (err->code) {
		ERROR("receive failed err code: %d msg: %s", err->code, err->msg);
		return err->code;
	}

	SUCCESS("result of kind %s", presult->kind);

	if (strcmp(presult->kind, "int") == 0) {
		SUCCESS("response: %d", presult->data.i);
	}

	return 0;
}

static int cmd_sendd(connection_t *conn, char* args) {

	comm_error_t *err;
	parse_result_t *presult;

	if (conn->state != CONNECTION_STATE_OPEN) {

		WARN("Please open connection first");

		return 1;
	}

	err = NEW(comm_error_t);

	send_int(atof(args), conn, err);

	if (err->code) {
		ERROR("send failed err code: %d msg: %s", err->code, err->msg);
		return err->code;
	}

	presult = receive(conn, err);

	if (err->code) {
		ERROR("receive failed err code: %d msg: %s", err->code, err->msg);
		return err->code;
	}

	SUCCESS("result of kind %s", presult->kind);

	if (strcmp(presult->kind, "double") == 0) {
		SUCCESS("response: %f", presult->data.d);
	}

	return 0;
}

static int cmd_get(connection_t *conn, char* arg_str) {

	command_get_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;
	int argc = 0;
	char ** argv;

	if (conn->state != CONNECTION_STATE_OPEN) {

		WARN("Please open connection first");

		return 1;
	}

	if(!logged){
		WARN("Please login first");

		return 1;
	}

	cmd = NEW(command_get_t);

	err = NEW(comm_error_t);

	argv = split_arguments(arg_str);
	argc = count_elements(argv);

	if(argc != 1){
		ERROR("TOO MUCH ARGUMENTS");
		return err->code = 10000;
	}

	cmd->path = argv[0];

	send_cmd_get(cmd, conn, err);

	if (err->code) {
		ERROR("send failed code %d msg: %s", err->code, err->msg);
		return err->code;
	}

	INFO("fetching");
	presult = receive(conn, err);
	INFO("fetched");

	if (err->code) {
		ERROR("receive failed err code: %d msg: %s", err->code, err->msg);
		return err->code;
	}

	SUCCESS("result of kind %s", presult->kind);

	if (strcmp(presult->kind, "data") == 0) {
		SUCCESS("response: %s", (char*)presult->data.data);
	}
	return 0;
}

static int cmd_post(connection_t *conn, char * args){
	
	command_post_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;
	int argc = 0;
	char ** argv;

	if (conn->state != CONNECTION_STATE_OPEN) {

		WARN("Please open connection first");

		return 1;
	}

	if(!logged){
		WARN("Please login first");

		return 1;
	}

	cmd = NEW(command_post_t);

	err = NEW(comm_error_t);

	argv = split_arguments(args);
	argc = count_elements(argv);

	if(argc != 2){
		ERROR("Correct use: post alias file");
		return err->code = 10001;
	}

	cmd->dest = argv[0];
	cmd->data = encode_raw_data(raw_data_from_file(argv[1], &cmd->size), cmd->size);
	 // = strlen(cmd->data);//TODO CHANGE

	send_cmd_post(cmd, conn, err);

	if (err->code) {
		ERROR("send failed code %d msg: %s", err->code, err->msg);
		return err->code;
	}

	INFO("fetching");
	presult = receive(conn, err);
	INFO("fetched");

	if (err->code) {
		ERROR("receive failed err code: %d msg: %s", err->code, err->msg);
		return err->code;
	}

	SUCCESS("result of kind %s", presult->kind);

	if (strcmp(presult->kind, "data") == 0) {
		SUCCESS("response: %s", (char*)presult->data.data);
	}

	return 0;
}

static int cmd_login(connection_t *conn, char * args){

	command_login_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;
	int argc = 0;
	char ** argv;

	if (conn->state != CONNECTION_STATE_OPEN) {

		WARN("Please open connection first");

		return 1;
	}

	cmd = NEW(command_login_t);
	cmd->user = NEW(user_t);

	err = NEW(comm_error_t);

	argv = split_arguments(args);
	argc = count_elements(argv);
	
	if(argc != 2){
		ERROR("Correct use: login username password");
		return err->code = 10001;
	}

	cmd->user->username = strdup(argv[0]);
	cmd->user->password = strdup(argv[1]);

	send_cmd_login(cmd, conn, err);

	if (err->code) {
		ERROR("send failed code %d msg: %s", err->code, err->msg);
		return err->code;
	}

	INFO("fetching");
	presult = receive(conn, err);
	INFO("fetched");

	if (err->code) {
		ERROR("receive failed err code: %d msg: %s", err->code, err->msg);
		return err->code;
	}

	SUCCESS("result of kind %s", presult->kind);

	if (strcmp(presult->kind, "data") == 0) {
		SUCCESS("response: %s", (char*)presult->data.data);
	}
	logged = yes;
	return 0;
}

static int cmd_logout(connection_t *conn, char * args){

	command_logout_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;
	int argc = 0;
	char ** argv;

	if (conn->state != CONNECTION_STATE_OPEN) {

		WARN("Please open connection first");
		return 1;
	}
	
	if(!logged){
		WARN("You have to be logged to logout");

		return 1;
	}

	argv = split_arguments(args);
	argc = count_elements(argv);
	
	if(argc != 0){
		ERROR("Correct use: logout");
		return err->code = 10002;
	}

	send_cmd_logout(cmd, conn, err);

	if (err->code) {
		ERROR("send failed code %d msg: %s", err->code, err->msg);
		return err->code;
	}

	INFO("fetching");
	presult = receive(conn, err);
	INFO("fetched");

	if (err->code) {
		ERROR("receive failed err code: %d msg: %s", err->code, err->msg);
		return err->code;
	}

	SUCCESS("result of kind %s", presult->kind);

	if (strcmp(presult->kind, "data") == 0) {
		SUCCESS("response: %s", (char*)presult->data.data);
	}
	logged = no;
	return 0;
}