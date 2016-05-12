#include <server_utilss.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <msqueue.h>
#include <mqueue.h>
mqd_t mq;

void log_mq(char *kind,char * str);
void init_mq(){
	if ((mq = mq_open(MSQUEUE_NAME, O_WRONLY))<0){
    	//AGREGAR EXECV
		printf("%d\n",errno);
		exit(-1);
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