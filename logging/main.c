#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "msqueue.h"

#include <errno.h>

mqd_t create_mq();
void read_mq(mqd_t mq, char* buff);
void close_mq(mqd_t mq);

int main(int argc, char const *argv[])
{
	mqd_t mq;
	int end_service = 0;
	char buff[MSG_MAX_SIZE+1];

	FILE* log_file = fopen("./server.log", "w+t");

	mq=create_mq();
    printf("Logging service launched successfully\n");


    int n = 0;
	while(!end_service){
		read_mq(mq, buff);
        if (!strncmp(buff, MSG_END, strlen(MSG_END)))
        {
            end_service = 1;
            printf("Logging service ended successfully\n");
        }
        else
        {
        	fputs(buff,log_file);
        }

	}

	close_mq(mq);

	fclose(log_file);
	return 0;
}

mqd_t create_mq(){

	mqd_t mq;

    struct mq_attr attr;

	/* init queue attributes */
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MSG;
    attr.mq_msgsize = MSG_MAX_SIZE;
    attr.mq_curmsgs = 0;

	if ((mq = mq_open(MSQUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr))== -1)
	{
		printf("%d\n",errno);
		exit(-1);
	}
	return mq;
}

void read_mq(mqd_t mq, char* buff){
	ssize_t bytes_readd;
	if((bytes_readd = mq_receive(mq, buff, MSG_MAX_SIZE, NULL)) < 0){
		/*print error? exit?*/
	}
	buff[bytes_readd]='\0';
}

void close_mq(mqd_t mq){
	mq_close(mq); /*-1 = error, print error?*/
	mq_unlink(MSQUEUE_NAME); /*-1 = error, print error?*/
}

