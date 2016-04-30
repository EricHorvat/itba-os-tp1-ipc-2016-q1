#include <commands.h>
#include <stdio.h>
#include <string.h>
#include <utils.h>

typedef struct {
	char *name;
	int (*cmd)(connection_t *, char *args);
	char *help;
} client_command_t;

static bool initialized_commands = no;

static void initialize_commands();

static int cmd_open(connection_t *, char *args);
static int cmd_sendi(connection_t *conn, char* args);
static int cmd_sendd(connection_t *conn, char* args);

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

	commands[i] = NULL;
}

int cmd_parse(connection_t *conn, char *cmd) {

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

					return commands[j]->cmd(conn, cmd+1);
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

		printf(ANSI_COLOR_YELLOW"Please open connection first\n"ANSI_COLOR_RESET);

		return 1;
	}

	err = NEW(comm_error_t);

	send_int(atoi(args), conn, COMMUNICATION_CLIENT_SERVER, err);

	if (err->code) {
		printf(ANSI_COLOR_RED"send failed err code: %d msg: %s\n"ANSI_COLOR_RESET, err->code, err->msg);
		return err->code;
	}

	presult = receive(conn, COMMUNICATION_SERVER_CLIENT, err);

	if (err->code) {
		printf(ANSI_COLOR_RED"receive failed err code: %d msg: %s\n"ANSI_COLOR_RESET, err->code, err->msg);
		return err->code;
	}

	printf("result of kind %s\n", presult->kind);

	if (strcmp(presult->kind, "int") == 0) {
		printf(ANSI_COLOR_GREEN"response: %d\n"ANSI_COLOR_RESET, presult->data.i);
	}

	return 0;
}

static int cmd_sendd(connection_t *conn, char* args) {

	comm_error_t *err;
	parse_result_t *presult;

	if (conn->state != CONNECTION_STATE_OPEN) {

		printf(ANSI_COLOR_YELLOW"Please open connection first\n"ANSI_COLOR_RESET);

		return 1;
	}

	err = NEW(comm_error_t);

	send_int(atof(args), conn, COMMUNICATION_CLIENT_SERVER, err);

	if (err->code) {
		printf(ANSI_COLOR_RED"send failed err code: %d msg: %s\n"ANSI_COLOR_RESET, err->code, err->msg);
		return err->code;
	}

	presult = receive(conn, COMMUNICATION_SERVER_CLIENT, err);

	if (err->code) {
		printf(ANSI_COLOR_RED"receive failed err code: %d msg: %s\n"ANSI_COLOR_RESET, err->code, err->msg);
		return err->code;
	}

	printf("result of kind %s\n", presult->kind);

	if (strcmp(presult->kind, "int") == 0) {
		printf(ANSI_COLOR_GREEN"response: %d\n"ANSI_COLOR_RESET, presult->data.i);
	}

	return 0;
}
