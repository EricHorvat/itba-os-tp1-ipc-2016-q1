#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>

#include "msqueue.h"

mqd_t open_mq();

int main(int argc, char **argv)
{
    mqd_t mq;
	char buff[MSG_MAX_SIZE+1];

    mq = open_mq();

    do {
        printf("> ");
        fflush(stdout);

        memset(buff, 0, MSG_MAX_SIZE);

        fgets(buff, MSG_MAX_SIZE, stdin);

        /* send the message */
        if(0 <= mq_send(mq, buff, MSG_MAX_SIZE, 0));

    } while (strncmp(buff, MSG_END, strlen(MSG_END)));

	
	mq_close(mq); /*-1 = error, print error?*/

    return 0;
}

mqd_t open_mq(){
    mqd_t mq;
    if ((mq = mq_open(MSQUEUE_NAME, O_WRONLY))<0)
    {
		printf("%d\n",errno);
		exit(-1);
	}
}