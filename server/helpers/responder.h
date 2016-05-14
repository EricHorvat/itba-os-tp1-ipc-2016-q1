#ifndef __RESPONDER_H__
#define __RESPONDER_H__

#include <serialization.h>

void process_get_cmd(connection_t* conn, command_get_t* cmd);
void process_post_cmd(connection_t* conn, command_post_t* post);
void process_login_cmd(connection_t* conn, command_login_t* login);
void process_logout_cmd(connection_t* conn);
void process_new_user_cmd(connection_t* conn, command_new_user_t* logout);

#endif  // __RESPONDER_H__
