#ifndef __RESPONDER_H__
#define __RESPONDER_H__

#include <serialization.h>

void process_get_cmd(connection_t* conn, command_get_t* cmd, comm_error_t* err);
void process_post_cmd(connection_t* conn, command_post_t* post, comm_error_t* err);
void process_login_cmd(connection_t* conn, command_login_t* login, comm_error_t* err);
void process_logout_cmd(connection_t* conn, comm_error_t* err);
void process_new_user_cmd(connection_t* conn, command_new_user_t* new_user, comm_error_t* err);
void process_change_pass_cmd(connection_t* conn, command_change_pass_t* change, comm_error_t* err);

#define ERR_OK 0

#define ERR_ASKED_FILE_IS_NOT_AVAILABLE 6000
#define ERR_USER_ALREADY_LOGGED 6001
#define ERR_USER_NOT_EXISTS 6002
#define ERR_USER_HAVE_NOT_PERMISSION 6003
#define ERR_USER_NOT_LOGGED 6004
#define ERR_USER_EXISTS 6005
#define ERR_FILE_EXISTS 6006

#endif  // __RESPONDER_H__
