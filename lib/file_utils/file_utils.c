#include <file_utils.h>
#include <stdlib.h>
#include <utils.h>
#include <string.h>

/**
 * checks whether a file exists
 * @param  path path to tell if exists
 * @return      0 if it exists -1 otherwise
 */
bool exists(char* path) {

	if (access(path, F_OK) == -1) {
		return no;
	}
	return yes;

}

void write_one_by_one_without_zero(int fd, void *data, size_t size) {
	size_t written = 0;
	while (written < size && *(char*)(data+written) != '\0') {
		write(fd, data+written, 1);
		written++;
	}
}

/**
 * writes data one byte at a time
 * @param fd   fd to write to
 * @param data data to write
 * @param size size of data
 */
void write_one_by_one(int fd, void *data, size_t size) {
	size_t written = 0;
	while (written < size && *(char*)(data+written) != '\0') {
		write(fd, data+written, 1);
		written++;
	}
	write(fd, "\0", 1);
}

void busy_wait_file_exists(char *path) {
	if (!exists(path)) {
		while (!exists(path)) {
			sleep(1);
		}
	}
}

char * raw_data_from_file(char * path, int *length){


	char * contents;
	FILE *file;
	long size;

	INFO("opening %s", path);
	if ( (file = fopen(path, "r")) == NULL) {
		ERROR("cant open file %s", path);
		return NULL;
	}
	fseek(file, 0, SEEK_END);
	size = ftell(file);
	fseek(file, 0, SEEK_SET);

	contents = (char*)malloc(size + 1);
	fread(contents, size, 1, file);
	fclose(file);

	contents[size] = 0;

	*length = size; 

	return contents;
}

char* encode_raw_data(char* data, int size) {

	unsigned char *output, *tmp;
	size_t wrote = 0;

	output = (char*)malloc(size*3+2);
	tmp = (char*)malloc(3);

	memset(output, ZERO, size*3+2);
	memset(tmp, ZERO, 3);

	while (wrote < size) {

		sprintf(tmp, "%03d", (unsigned int)*(unsigned char*)(data+wrote));
		strncat(output, tmp, 3);

		wrote++;
	}

	free(tmp);

	return output;

}

char* decode_to_raw_data(char* data) {

	char *result;
	size_t len;
	int i = 0;
	int ascii;

	INFO("decode %s", data);

	len = strlen(data);

	result = (char*)malloc(len/3+1);

	INFO("decode %s", data);

	for (; i < len; i+=3) {
		ascii = (data[i] - '0')*100 + (data[i+1] - '0')*10 + (data[i+2] - '0');
		result[i/3] = ascii;
	}

	result[i] = '\0';

	return result;

}

int file_from_row_data(char * path, char * data, size_t size){

	FILE *file;
	int written_bytes;

	INFO("opening %s", path);
	if ( (file = fopen(path, "w")) == NULL) {
		ERROR("cant open file %s", path);
		return -1;
	}
	INFO("opened, to write");
	written_bytes = 0;
	do{
		written_bytes += fwrite(data+written_bytes,1,size - written_bytes,file);
	}while(written_bytes < size);
	INFO("ENDING");
	fclose(file);

	return 0;
}
