#ifndef __RESPONDER_H__
#define __RESPONDER_H__

#include <serialization.h>

void process_get_cmd(connection_t *conn, command_get_t *cmd);
void process_post_cmd(connection_t *conn, command_post_t *post);

#endif // __RESPONDER_H__
