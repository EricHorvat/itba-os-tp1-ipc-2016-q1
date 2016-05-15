#ifdef __LOGGING__
#include <helpers/logging_helpers.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msqueue.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/wait.h>
#include <utils.h>
#include <errno.h>
#include <unistd.h>

#define LOGGING_BINARY_DIR "./logging.bin"

static mqd_t mq;

static void init_logging_server(void);
static int  tried_to_start_server = 0;

void log_mq(char* kind, char* str);

void init_mq(void) {
	init_logging_server();		
	if ((mq = mq_open(MSQUEUE_NAME, O_WRONLY)) < 0) {
			printf("%d\n", errno);
			exit(-1);
	}
}

void log_error(char* str) {
	log_mq("ERROR", str);
}
void log_info(char* str) {
	log_mq("INFO", str);
}
void log_warning(char* str) {
	log_mq("WARNING", str);
}

void log_success(char* str) {
	log_mq("SUCCESS", str);
}

void log_mq(char* kind, char* str) {
	char* msg_f;

	INFO("logging to message queue");

	msg_f = (char*)malloc(strlen(kind) + 2 + strlen(str) + 2);
	sprintf(msg_f, "%s: %s\n", kind, str);
	if ( mq_send(mq, strdup(msg_f), strlen(msg_f), 0) != 0 ) {
		ERROR("msq failed with error %d\tmsg:%s", errno, strerror(errno));
	}
}

static void init_logging_server(void) {
	pid_t    child_pid = 0;
	char **args, **envp;

	if ((child_pid = fork()) == 0) {

		// child

		args    = malloc(sizeof(char*) * 2);
		envp    = malloc(sizeof(char*));
		envp[0] = NULL;
		args[0] = LOGGING_BINARY_DIR;
		args[1] = NULL;
		execve(LOGGING_BINARY_DIR, args, envp);

	} else if (child_pid < 0) {
		ERROR("CANT FORK\n");
	}
	sleep(1);
}
#endif
