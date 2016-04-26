#ifndef __UTILS_H_
#define __UTILS_H_

typedef unsigned char bool;
typedef unsigned char boolean;

#define TRUE 1
#define FALSE 0

#define true 1
#define false 0

#define YES 1
#define NO 0

#define yes 1
#define no 0

#define null NULL
#define nil NULL

typedef char* string;

#define NEW(type) (type*)malloc(sizeof(type))

#define NEWLINE '\n'
#define ZERO '\0'
#define SLASH '/'
#define COLON ':'

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#endif