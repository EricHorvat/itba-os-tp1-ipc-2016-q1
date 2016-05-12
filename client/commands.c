#include <commands.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <stdlib.h>
#include <file_utils.h>

typedef struct {
	char *correct_use;
	char *help;
} client_command_info_t;

typedef struct {
	char *name;
	int (*cmd)(connection_t *, client_command_info_t * info, char ** argv, int argc);
	client_command_info_t * info;
} client_command_t;

static bool initialized_commands = no;
static bool logged = no;

static void initialize_commands();
static client_command_t * new_command(char * name, char * correct_use, char * help, int (*cmd)(connection_t *, client_command_info_t * info, char ** argv, int argc));
static bool show_help_for_command(char* arg, client_command_info_t * info);

static int cmd_open(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_sendi(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_sendd(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_sends(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_get(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_post(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_login(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_logout(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_close(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_commands(connection_t *conn, client_command_info_t * info, char ** argv, int argc);
static int cmd_help(connection_t *conn, client_command_info_t * info, char ** argv, int argc);

static client_command_t **commands;

static void initialize_commands() {

	int i = 0;

	commands = (client_command_t**)malloc(50*sizeof(client_command_t*));

	commands[i++] = new_command(strdup("open"),"open url","CAMopen help", &cmd_open);
	commands[i++] = new_command("close","close","CAMclose help", &cmd_close);
	commands[i++] = new_command("sendi","sendi int","CAMsendi help", &cmd_sendi);
	commands[i++] = new_command("sendd","sendd double","CAMsendd help", &cmd_sendi);
	commands[i++] = new_command("sends","sends string","CAMsends help", &cmd_sends);
	commands[i++] = new_command("help","help command","CAMhelp help", &cmd_help);
	commands[i++] = new_command("get","get alias","CAMget help", &cmd_get);
	commands[i++] = new_command("post","post alias file","CAMpost help", &cmd_post);
	commands[i++] = new_command("login","login username password","CAMlogin help", &cmd_login);
	commands[i++] = new_command("logout","logout","CAMlogout help", &cmd_logout);
	commands[i++] = new_command("commands","commands","CAMcommands help", &cmd_commands);
	commands[i] = NULL;
}


static client_command_t * new_command(char * name, char * correct_use, char * help 
		, int (*cmd)(connection_t *,client_command_info_t * info,  char ** argv, int argc)){
	
	client_command_t * command;
	command = NEW(client_command_t);
	command->info = NEW(client_command_info_t);
	command->name = strdup(name);
	command->cmd = cmd;
	command->info->correct_use = correct_use;
	command->info->help = help;
	return command;
}

static bool show_help_for_command(char* arg, client_command_info_t * info){

	if(strcmp("help", arg)==0){
		LOG("%s",info->help);
		return yes;
	}
	return no;
}

int cmd_parse(connection_t *conn, char * cmd) {

	char ** comms;
	int argc;
	int i = 0, j = 0;

	if (!initialized_commands) {
		initialize_commands();
		initialized_commands = yes;
	}

	comms = split_arguments(cmd);
	argc = count_elements(comms);

	while (commands[j] != NULL) {

		if (strcmp(comms[0], commands[j]->name) == 0) {
			int error_code;
			error_code = commands[j]->cmd(conn, commands[j]->info, comms+1,argc-1);
			return error_code;
		}
		j++;
	}
	printf(ANSI_COLOR_YELLOW"[%s] Command not found\n"ANSI_COLOR_RESET, comms[0]);
	return 1;


}

static int cmd_open(connection_t *conn, client_command_info_t * info, char ** argv, int argc) {

	int conn_error = -4;

	conn->server_addr = NEW(comm_addr_t);
	
	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;
	
	if (isConnectionOpen(conn)) {

		WARN("Opened connection");

		return 1;
	}

	if(argc != 1){
		ERROR("Correct use: %s",info->correct_use);
		return 1;
	}
	
	if (address_from_url(argv[0], conn->server_addr)) {
		return 5;
	}

	if ( (conn_error = connection_open(conn) != 0) ) {
		fprintf(stderr, ANSI_COLOR_RED"error %d creating connection\n"ANSI_COLOR_RESET, conn_error);
		return conn_error;
	}

	return 0;

}

static int cmd_sendi(connection_t *conn, client_command_info_t * info, char ** argv, int argc) {

	comm_error_t *err;
	parse_result_t *presult;

	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;
	
	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");

		return 1;
	}

	if(argc != 1){
		ERROR("Correct use: %s",info->correct_use);
		return 1;

	}
	
	err = NEW(comm_error_t);

	INFO("sending %s", argv[0]);

	send_int(atoi(argv[0]), conn, err);

	INFO("sent %s", argv[0]);

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

static int cmd_sends(connection_t *conn, client_command_info_t * info, char ** argv, int argc) {

	comm_error_t *err;
	parse_result_t *presult;

	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;
	
	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");

		return 1;
	}

	if(argc != 1){
		ERROR("Correct use: %s",info->correct_use);
		return 1;
	}
	
	err = NEW(comm_error_t);

	INFO("sending %s", argv[0]);

	send_string(argv[0], conn, err);

	INFO("sent %s", argv[0]);

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

	if (strcmp(presult->kind, "string") == 0) {
		SUCCESS("response: %s", presult->data.str);
	}

	return 0;
}

static int cmd_sendd(connection_t *conn, client_command_info_t * info, char ** argv, int argc) {

	comm_error_t *err;
	parse_result_t *presult;

	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;	

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");

		return 1;
	}
	
	err = NEW(comm_error_t);

	send_int(atof(argv[0]), conn, err);

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

static int cmd_get(connection_t *conn, client_command_info_t * info, char ** argv, int argc) {

	command_get_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;
	
	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;
	
	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");

		return 1;
	}

	if(!logged){
		WARN("Please login first");

		return 1;
	}

	cmd = NEW(command_get_t);

	err = NEW(comm_error_t);

	if(argc != 1){
		ERROR("Correct use: %s",info->correct_use);
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

static int cmd_post(connection_t *conn, client_command_info_t * info, char ** argv, int argc){
	
	command_post_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;
	
	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");

		return 1;
	}

	if(!logged){
		WARN("Please login first");

		return 1;
	}

	cmd = NEW(command_post_t);

	err = NEW(comm_error_t);

	if(argc != 2){
		
		ERROR("Correct use: %s",info->correct_use);
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

static int cmd_login(connection_t *conn, client_command_info_t * info, char ** argv, int argc){

	command_login_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;

	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");

		return 1;
	}

	cmd = NEW(command_login_t);
	cmd->user = NEW(user_t);

	err = NEW(comm_error_t);

	if(argc != 2){
		ERROR("Correct use: %s",info->correct_use);
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

static int cmd_logout(connection_t *conn, client_command_info_t * info, char ** argv, int argc){

	command_logout_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;

	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		return 1;
	}
	
	if(!logged){
		WARN("You have to be logged to logout");

		return 1;
	}

	if(argc != 0){	
		ERROR("Correct use: %s",info->correct_use);
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

static int cmd_close(connection_t *conn, client_command_info_t * info, char ** argv, int argc){

	command_close_t *cmd;
	comm_error_t *err;
	parse_result_t *presult;

	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		return 1;
	}

	if(logged){
		WARN("You have not to be logged to close");
		return 1;
	}

	if(argc != 0){
		ERROR("Correct use: %s",info->correct_use);
		return err->code = 10002;
	}

	
	send_cmd_close(cmd, conn, err); //-> should set CONNECTION_STATE_CLOSED, close only one fifo and exit 

	conn->state = CONNECTION_STATE_CLOSED;

	if (err->code) {
		ERROR("send failed code %d msg: %s", err->code, err->msg);
		return err->code;
	}

	return 0;
}

static int cmd_commands(connection_t *conn, client_command_info_t * info, char** argv, int argc){

	int i = 0;

	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;
	while(commands[i] != NULL){

		printf("%s\n", commands[i]->name);
		i++;
	}
	return 0;
}

static int cmd_help(connection_t *conn, client_command_info_t * info, char** argv, int argc){

	int i = 0;
	comm_error_t *err;
	
	if(argc == 1 && show_help_for_command(argv[0],info)) return 0;
	
	if(argc != 1){
		ERROR("Correct use: %s",info->correct_use);
		return err->code = 10002;
	}

	while(commands[i] != NULL){
		if(strcmp(commands[i]->name,argv[0])==0){
			printf("%s\n", commands[i]->info->correct_use);
		}
		i++;
	}
	return 0;
}

bool isConnectionOpen(connection_t * conn){
	return conn->state == CONNECTION_STATE_OPEN;  
}

bool isConnectionClosed(connection_t * conn){
	return conn->state == CONNECTION_STATE_CLOSED;  
}