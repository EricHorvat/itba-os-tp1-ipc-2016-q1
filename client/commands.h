#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#include <serialization.h>

int cmd_parse(connection_t *, char*);

bool isConnectionOpen(connection_t * conn);
bool isConnectionClosed(connection_t * conn);

#define COMMAND_OK 0
#define ERR_COMMAND_NOT_FOUND 100
#define ERR_ALREADY_DONE 101
#define ERR_CONNECTION_NOT_OPEN 102
#define ERR_WRONG_ARGUMENTS_COUNT 103
#define ERR_CONNECTION_NOT_LOGGED 104
#define ERR_MISUSE_ARGUMENT 105
#define ERR_BAD_ADDRESS 106


#endif