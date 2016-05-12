#include <server_utilss.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msqueue.h>
#include <mqueue.h>
mqd_t mq;

#define LOGGING_BINARY_DIR "./logging.bin"

void init_logging_server();
int tried_to_start_server = 0;

void log_mq(char *kind,char * str);
void init_mq(){
	int child_pid = 0;
	if ((mq = mq_open(MSQUEUE_NAME, O_WRONLY))<0){
    	//SERVER DE LOGGING APAGADO
		if(!tried_to_start_server){
			init_logging_server();
			tried_to_start_server++;
			init_mq();
		}else{
		printf("%d\n",errno);
		exit(-1);
		}
	}
}

void log_error(char *str){
	log_mq("ERROR",str);
}
void log_info(char *str){
	log_mq("INFO",str);
}
void log_warning(char *str){
	log_mq("WARNING",str);
}

void log_mq(char *kind,char * str){
	char * msg_f = malloc (sizeof(char)*(strlen(kind)+2+strlen(str)+2));
	sprintf(msg_f,"%s: %s\n",kind,str);
	if (mq_send(mq, strdup(msg_f), strlen(msg_f), 0) != 0) {
		printf("msg q");
	}
}

void init_logging_server(){
	int child_pid = 0;
	if((child_pid = fork())==0){
		char ** args, ** envp;
		args = malloc(sizeof(char*)*2);
		envp = malloc(sizeof(char*));
		envp[0]=NULL;
		args[0]=LOGGING_BINARY_DIR;
		args[1]=NULL;
		execve(LOGGING_BINARY_DIR,args,envp);
	}else if( child_pid < 0){
		printf("CANT FORK\n");
		return;
	}
	waitpid(child_pid);
}