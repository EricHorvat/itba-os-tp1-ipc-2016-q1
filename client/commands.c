#include <commands.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>
#include <stdlib.h>
#include <file_utils.h>
#include <errno.h>

typedef struct {
	char* correct_use;
	char* help;
} client_command_info_t;

typedef struct {
	char* name;
	int (*cmd)(connection_t*, client_command_info_t* info, char** argv, int argc);
	client_command_info_t* info;
} client_command_t;

static bool initialized_commands = no;
static bool logged               = no;

static void              initialize_commands(void);
static client_command_t* new_command(char* name, char* correct_use, char* help, int (*cmd)(connection_t*, client_command_info_t* info, char** argv, int argc));
static bool show_help_for_command(char* arg, client_command_info_t* info);

static int cmd_open(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_sendi(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_sendd(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_sends(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_get(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_post(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_login(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_logout(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_close(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_commands(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_help(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_new_user(connection_t* conn, client_command_info_t* info, char** argv, int argc);
static int cmd_change_pass(connection_t* conn, client_command_info_t* info, char** argv, int argc);

static client_command_t** commands;

static void initialize_commands(void) {

	int i = 0;

	commands = (client_command_t**)malloc(50 * sizeof(client_command_t*));

	commands[i++] = new_command(strdup("open"), "open url", "CAMopen help", &cmd_open);
	commands[i++] = new_command("close", "close", "CAMclose help", &cmd_close);
	commands[i++] = new_command("sendi", "sendi int", "CAMsendi help", &cmd_sendi);
	commands[i++] = new_command("sendd", "sendd double", "CAMsendd help", &cmd_sendd);
	commands[i++] = new_command("sends", "sends string", "CAMsends help", &cmd_sends);
	commands[i++] = new_command("help", "help command", "CAMhelp help", &cmd_help);
	commands[i++] = new_command("get", "get alias", "CAMget help", &cmd_get);
	commands[i++] = new_command("post", "post alias file", "CAMpost help", &cmd_post);
	commands[i++] = new_command("login", "login username password", "CAMlogin help", &cmd_login);
	commands[i++] = new_command("logout", "logout", "CAMlogout help", &cmd_logout);
	commands[i++] = new_command("commands", "commands", "CAMcommands help", &cmd_commands);
	commands[i++] = new_command("newuser", "newuser name yes/no [pass]", "CAMnewuser help", &cmd_new_user);
	commands[i++] = new_command("ch_password", "ch_password pass", "CAMch_password help", &cmd_change_pass);
	commands[i]   = NULL;
}

static client_command_t* new_command(char* name, char* correct_use, char* help, int (*cmd)(connection_t*, client_command_info_t* info, char** argv, int argc)) {

	client_command_t* command;
	command                    = NEW(client_command_t);
	command->info              = NEW(client_command_info_t);
	command->name              = strdup(name);
	command->cmd               = cmd;
	command->info->correct_use = correct_use;
	command->info->help        = help;
	return command;
}

static bool show_help_for_command(char* arg, client_command_info_t* info) {

	if (strcmp("help", arg) == 0) {
		LOG("%s", info->help);
		return yes;
	}
	return no;
}

int cmd_parse(connection_t* conn, char* cmd) {

	char** comms;
	int    argc;
	int    j = 0;

	if (!initialized_commands) {
		initialize_commands();
		initialized_commands = yes;
	}

	comms = split_arguments(cmd);
	argc  = count_elements(comms);

	while (commands[j] != NULL) {

		if (strcmp(comms[0], commands[j]->name) == 0) {
			int error_code;
			error_code = commands[j]->cmd(conn, commands[j]->info, comms + 1, argc - 1);
			return error_code;
		}
		j++;
	}
	printf(ANSI_COLOR_YELLOW "[%s] Command not found\n" ANSI_COLOR_RESET, comms[0]);
	return ERR_COMMAND_NOT_FOUND;
}

static int cmd_open(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	int conn_error;

	conn->server_addr = NEW(comm_addr_t);

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (isConnectionOpen(conn)) {

		WARN("Opened connection");

		return ERR_ALREADY_DONE;
	}

	if (argc != 1) {
		ERROR("Correct use: %s", info->correct_use);
		return ERR_WRONG_ARGUMENTS_COUNT;
	}

	if (address_from_url(argv[0], conn->server_addr) != ADDRESS_OK) {
		return ERR_BAD_ADDRESS;
	}

	if ((conn_error = connection_open(conn) != 0)) {
		fprintf(stderr, ANSI_COLOR_RED "error %d creating connection\n" ANSI_COLOR_RESET, conn_error);
		return conn_error;//CHANGE IN CONNECTION OPEN
	}

	return COMMAND_OK;
}

static int cmd_sendi(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	comm_error_t*   err;
	parse_result_t* presult;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg="Connection not opened";
		return err->code=ERR_CONNECTION_NOT_OPEN;
	}

	if (argc != 1) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg = "More or less arguments";
		return err->code=ERR_WRONG_ARGUMENTS_COUNT;
	}

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

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

	return COMMAND_OK;
}

static int cmd_sends(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	comm_error_t*   err;
	parse_result_t* presult;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg="Connection not opened";
		return err->code=ERR_CONNECTION_NOT_OPEN;
	
	}

	if (argc != 1) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg = "More or less arguments";
		return err->code=ERR_WRONG_ARGUMENTS_COUNT;
	}

	err = NEW(comm_error_t);

	INFO("sending %s", argv[0]);
	err->msg="Sending command";
	err->code=COMMAND_OK;

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

	return COMMAND_OK;
}

static int cmd_sendd(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	comm_error_t*   err;
	parse_result_t* presult;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	
	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg="Connection not opened";
		return err->code=ERR_CONNECTION_NOT_OPEN;
	
	}

	if (argc != 1) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg = "More or less arguments";
		return err->code=ERR_WRONG_ARGUMENTS_COUNT;
	}

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

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

	return COMMAND_OK;
}

static int cmd_get(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	command_get_t*  cmd;
	comm_error_t*   err;
	parse_result_t* presult;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg="Connection not opened";
		return err->code=ERR_CONNECTION_NOT_OPEN;
	}

	if (!logged) {
		WARN("Please login first");
		err->msg = "Not logged";
		return err->code = ERR_CONNECTION_NOT_LOGGED;
	}

	cmd = NEW(command_get_t);

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

	if (argc != 1) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg = "More or less arguments";
		return err->code = ERR_WRONG_ARGUMENTS_COUNT;
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
	return COMMAND_OK;
}

static int cmd_post(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	command_post_t* cmd;
	comm_error_t*   err;
	parse_result_t* presult;
	char * aux;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg = "Connection not opened";
		return err->code = ERR_CONNECTION_NOT_OPEN;
	}

	if (!logged) {
		WARN("Please login first");
		err->msg = "Not logged";
		return ERR_CONNECTION_NOT_LOGGED;
	}

	cmd = NEW(command_post_t);

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

	if (argc != 2) {

		ERROR("Correct use: %s", info->correct_use);
		err->msg="More or less arguments";
		return err->code = ERR_WRONG_ARGUMENTS_COUNT;
	}

	cmd->dest = argv[0];
	aux = raw_data_from_file(argv[1], &cmd->size);
	if (aux == NULL)
	{
		if(errno == ERR_FILE_NOT_OPENED){
			ERROR("Cant open file");
			err->msg="Can\'t open file";
		}
		return err->code = errno;
	}
	cmd->data = encode_raw_data(aux, cmd->size);

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

	return COMMAND_OK;
}

static int cmd_login(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	command_login_t* cmd;
	comm_error_t*    err;
	parse_result_t*  presult;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg = "Connection not opened";
		return err->code = ERR_CONNECTION_NOT_OPEN;
	}

	cmd       = NEW(command_login_t);
	cmd->user = NEW(user_t);

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

	if (argc != 2) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg = "More or less arguments";
		return err->code = ERR_WRONG_ARGUMENTS_COUNT;
	}

	if (logged) {
		WARN("You are already logged");
		err->msg="Already logged";
		return ERR_ALREADY_DONE;
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
	return COMMAND_OK;
}

static int cmd_logout(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	comm_error_t*   err;
	parse_result_t* presult;

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg = "Connection not opened";
		return err->code = ERR_CONNECTION_NOT_OPEN;
	}

	if (!logged) {
		WARN("You have to be logged to logout");
		err->msg = "Not logged";
		return err->code = ERR_CONNECTION_NOT_LOGGED;
	}

	if (argc != 0) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg="More or less arguments";
		return err->code = ERR_WRONG_ARGUMENTS_COUNT;
	}

	send_cmd_logout(conn, err);

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
	return COMMAND_OK;
}

static int cmd_new_user(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	command_new_user_t* cmd;
	comm_error_t*       err;
	parse_result_t*     presult;
	err       = NEW(comm_error_t);
	cmd       = NEW(command_new_user_t);
	cmd->user = NEW(user_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg = "Connection not opened";
		return err->code = ERR_CONNECTION_NOT_OPEN;
	}

	if (!logged) {
		WARN("You have to be logged");
		err->msg = "Not logged";
		return err->code = ERR_CONNECTION_NOT_LOGGED;
	}

	if (argc < 2 || argc > 3) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg = "More or less arguments";
		return err->code = ERR_WRONG_ARGUMENTS_COUNT;
	}

	cmd->user->username = strdup(argv[0]);
	if (strcmp(argv[1], "yes") == 0) {
		cmd->user->admin = yes;
	} else if (strcmp(argv[1], "no") == 0) {
		cmd->user->admin = yes;
	} else {
		ERROR("Correct use: %s", info->correct_use);
		err->msg="Misuse argument";
		return err->code = ERR_MISUSE_ARGUMENT;
	}

	if (argc == 2) {
		cmd->user->password = "pass123";
	} else {
		cmd->user->password = strdup(argv[2]);
	}

	send_cmd_new_user(cmd, conn, err);

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
	return COMMAND_OK;
}

static int cmd_close(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	comm_error_t* err;

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {

		WARN("Please open connection first");
		err->msg = "Connection not opened";
		return err->code = ERR_CONNECTION_NOT_OPEN;
	}

	if (logged) {
		WARN("You have not to be logged to close");
		err->msg = "Have not to be logged to close connection";
		return err->code = ERR_ALREADY_DONE;
	}

	if (argc != 0) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg = "More or less arguments";
		return err->code = ERR_WRONG_ARGUMENTS_COUNT;
	}

	send_cmd_close(conn, err);  //-> should set CONNECTION_STATE_CLOSED, close only one fifo and exit

	if (err->code) {
		ERROR("send failed code %d msg: %s", err->code, err->msg);
		return err->code;
	}

	return COMMAND_CLOSE_OK;
}

static int cmd_commands(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	int i = 0;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;
	while (commands[i] != NULL) {

		printf("%s\n", commands[i]->name);
		i++;
	}
	return COMMAND_OK;
}

static int cmd_help(connection_t* conn, client_command_info_t* info, char** argv, int argc) {

	int           i = 0;
	comm_error_t* err;

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (argc != 1) {
		ERROR("Correct use: %s", info->correct_use);
		err->msg = "More or less arguments";
		return err->code = ERR_WRONG_ARGUMENTS_COUNT;
	}

	while (commands[i] != NULL) {
		if (strcmp(commands[i]->name, argv[0]) == 0) {
			printf("%s\n", commands[i]->info->correct_use);
		}
		i++;
	}
	return COMMAND_OK;
}

static int cmd_change_pass(connection_t* conn, client_command_info_t* info, char** argv, int argc){
	command_change_pass_t* cmd;
	comm_error_t*   err;
	parse_result_t* presult;
	char * aux;

	if (argc == 1 && show_help_for_command(argv[0], info))
		return COMMAND_OK;

	if (!isConnectionOpen(conn)) {
		WARN("Please open connection first");
		err->msg = "Connection not opened";
		return err->code = ERR_CONNECTION_NOT_OPEN;
	}

	if (!logged) {
		WARN("Please login first");
		err->msg = "Not logged";
		return ERR_CONNECTION_NOT_LOGGED;
	}

	cmd = NEW(command_post_t);

	err = NEW(comm_error_t);
	err->msg="Sending command";
	err->code=COMMAND_OK;

	if (argc != 1) {

		ERROR("Correct use: %s", info->correct_use);
		err->msg="More or less arguments";
		return err->code = ERR_WRONG_ARGUMENTS_COUNT;
	}

	cmd->pass=strdup(argv[0]);

	send_cmd_change_pass(cmd, conn, err);

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

	return COMMAND_OK;
}