#include <file_utils.h>

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
	while (written < size) {
		write(fd, data+written, 1);
		written++;
	}
}