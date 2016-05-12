#ifndef __COMMANDS_TYPES_H__
#define __COMMANDS_TYPES_H__

#include <utils.h>
#include <stdlib.h>

typedef struct {
	char *path;
} command_get_t;

typedef struct {
	char *data;
	char *dest;
	size_t size;
} command_post_t;

typedef struct {
	char *username;
	char *password;
	bool admin;
} user_t;

typedef struct {
	user_t *user;
} command_login_t;

typedef struct {
} command_logout_t;

typedef struct {
} command_close_t;

typedef struct {
	user_t *user;
} command_new_user_t;

#endif