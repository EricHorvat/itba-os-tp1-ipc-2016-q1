#ifndef __SERVER_UTILS__
#define __SERVER_UTILS__

#define MAX_LOG_LENGHT 100
#define NEW_LOG_STR() malloc(sizeof(char)*MAX_LOG_LENGHT);

void init_mq();

void log_info(char * msg);

void log_error(char * msg);

void log_warning(char *msg);

#endif