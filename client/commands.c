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
static int sendi(connection_t *conn, char* args);

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
	commands[i]->cmd = &cmd_open;
	commands[i++]->help = "Sendi help";

	commands[i] = NEW(client_command_t);
	commands[i]->name = "sendd";
	commands[i]->cmd = &cmd_open;
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
	}

	return 0;

}

static int sendi(connection_t *conn, char* args) {


	send_int(atoi(args), conn, COMMUNICATION_CLIENT_SERVER, nil);

	return 0;
}
