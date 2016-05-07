#include <file_utils.h>
#include <stdlib.h>
#include <utils.h>

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

char * raw_data_from_file(char * path){


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

	return contents;
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
