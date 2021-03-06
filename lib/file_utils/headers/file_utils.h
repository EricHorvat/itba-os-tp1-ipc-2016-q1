#ifndef __FILE_UTILS_H__
#define __FILE_UTILS_H__

#include <unistd.h>
#include <utils.h>
#include <stdio.h>

boolean exists(char* path);
void write_one_by_one(int fd, void* data, size_t size);
void write_one_by_one_without_zero(int fd, void* data, size_t size);
void busy_wait_file_exists(char* path);
char* raw_data_from_file(char* path, size_t* length);
int file_from_raw_data(char* path, char* data, size_t size);

char* encode_raw_data(char* data, size_t size);
char* decode_to_raw_data(char* data);

#define ERR_FILE_NOT_OPENED 9000
#define FILE_OK 0

#endif
