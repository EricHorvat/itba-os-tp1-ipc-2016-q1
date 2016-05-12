#ifndef __CLIENT_COMMANDS_H__
#define __CLIENT_COMMANDS_H__

#include <serialization.h>

int cmd_parse(connection_t *, char*);

bool isConnectionOpen(connection_t * conn);
bool isConnectionClosed(connection_t * conn);



#endif