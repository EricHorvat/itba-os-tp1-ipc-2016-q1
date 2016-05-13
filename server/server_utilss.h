#ifndef __SERVER_UTILS__
#define __SERVER_UTILS__

#define MAX_LOG_LENGHT 100

void init_mq();

void log_info(char * msg);

void log_error(char * msg);

void log_warning(char *msg);

#endif