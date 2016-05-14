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

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_BROWN "\x1b[33m"
#define ANSI_COLOR_BLUE "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN "\x1b[36m"

#define ANSI_COLOR_DARK_GRAY "\x1b[1;30m"
#define ANSI_COLOR_LIGHT_RED "\x1b[1;31m"
#define ANSI_COLOR_LIGHT_GREEN "\x1b[1;32m"
#define ANSI_COLOR_YELLOW "\x1b[1;33m"
#define ANSI_COLOR_LIGHT_BLUE "\x1b[1;34m"
#define ANSI_COLOR_LIGHT_PURPLE "\x1b[1;35m"
#define ANSI_COLOR_LIGHT_CYAN "\x1b[1;35m"

#define ANSI_COLOR_RESET "\x1b[0m"

// Logging

#ifdef __DEBUG__

#define DEBUG_STR ANSI_COLOR_DARK_GRAY "%s:%d " ANSI_COLOR_RESET ANSI_COLOR_MAGENTA "<%s>:" ANSI_COLOR_RESET "\t"

#define LOG(msg, ...) printf(DEBUG_STR msg "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define INFO(msg, ...) printf(DEBUG_STR ANSI_COLOR_CYAN msg ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define WARN(msg, ...) printf(DEBUG_STR ANSI_COLOR_YELLOW msg ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define ERROR(msg, ...) fprintf(stderr, DEBUG_STR ANSI_COLOR_RED msg ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)
#define SUCCESS(msg, ...) printf(DEBUG_STR ANSI_COLOR_GREEN msg ANSI_COLOR_RESET "\n", __FILE__, __LINE__, __FUNCTION__, ##__VA_ARGS__)

#else

#define LOG(msg, ...) printf(msg, __VA_ARGS__)
#define INFO(msg, ...) printf(ANSI_COLOR_CYAN msg ANSI_COLOR_RESET "\n", ##__VA_ARGS__)
#define WARN(msg, ...) printf(ANSI_COLOR_YELLOW msg ANSI_COLOR_RESET "\n", ##__VA_ARGS__)
#define ERROR(msg, ...) fprintf(stderr, ANSI_COLOR_RED msg ANSI_COLOR_RESET "\n", ##__VA_ARGS__)
#define SUCCESS(msg, ...) printf(ANSI_COLOR_GREEN msg ANSI_COLOR_RESET "\n", ##__VA_ARGS__)

#endif

int    getrnd(int, int);
char*  gen_boundary(void);
char** split_arguments(char* sentence);
char** add(char* str, char** str_vector, int cant);
int count_elements(char** vector);

#endif