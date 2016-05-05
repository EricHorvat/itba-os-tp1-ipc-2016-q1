#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include <unistd.h>
#include <utils.h>
#include <stdio.h>

bool exists(char* path);
void write_one_by_one(int fd, void *data, size_t size);
void busy_wait_file_exists(char *path);
char * raw_data_from_file(char * path);

#endif