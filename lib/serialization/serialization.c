#include "headers/serialization.h"

#include "../../utils.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

static char* stringify_int(int);
static char* stringify_float(float);
static char* stringify_double(double);
static char* stringify_char(char);
static char* stringify_string(char*);

// Stringify

static const int BASE_LENGTH = 18;
static const int INT_LENGTH = 3;
static const int FLOAT_LENGTH = 5;
static const int DOUBLE_LENGTH = 6;
static const int CHAR_LENGTH = 4;
static const int STRING_LENGTH = 6;

static char* stringify_int(int i) {

	char *result;
	char int_str[5];

	sprintf(int_str, "%d", i);

	result = malloc(BASE_LENGTH+INT_LENGTH+strlen(int_str)+1);

	result[0] = ZERO;

	strcat(result, "{\"kind\":\"");
	strcat(result, "int\",value:");
	strcat(result, int_str);
	strcat(result, "}");

	printf("%s\n", result);

	return result;
}

static char* stringify_float(float i) {
	return "";
}

static char* stringify_double(double i) {
	return "";
}

static char* stringify_char(char c) {
	return "";
}

static char* stringify_string(char* s) {
	return "";
}

// Sync

void send_string(char *string, comm_addr_t *addr, comm_error_t *error) {
	char* serialized = stringify_string(string);
	send_data((void*)serialized, strlen(serialized), addr, error);
}

void send_int(int number, comm_addr_t *addr, comm_error_t *error) {
	char* serialized = stringify_int(number);
	send_data((void*)serialized, strlen(serialized), addr, error);
}

void send_float(float number, comm_addr_t *addr, comm_error_t *error) {
	string serialized = stringify_float(number);
	send_data((void*)serialized, strlen(serialized), addr, error);
}

void send_double(double number, comm_addr_t *addr, comm_error_t *error) {
	char* serialized = stringify_double(number);
	send_data((void*)serialized, strlen(serialized), addr, error);
}

void send_char(char character, comm_addr_t *addr, comm_error_t *error) {
	char* serialized = stringify_char(character);
	send_data((void*)serialized, strlen(serialized), addr, error);
}

// Async

void send_string_async(char *string, comm_addr_t *addr, comm_callback_t cb) {
	char* serialized = stringify_string(string);
	send_data_async((void*)serialized, strlen(serialized), addr, cb);
}

void send_int_async(int number, comm_addr_t *addr, comm_callback_t cb) {
	char* serialized = stringify_int(number);
	send_data_async((void*)serialized, strlen(serialized), addr, cb);
}

void send_float_async(float number, comm_addr_t *addr, comm_callback_t cb) {
	char* serialized = stringify_float(number);
	send_data_async((void*)serialized, strlen(serialized), addr, cb);
}

void send_double_async(double number, comm_addr_t *addr, comm_callback_t cb) {
	char* serialized = stringify_double(number);
	send_data_async((void*)serialized, strlen(serialized), addr, cb);
}

void send_char_async(char character, comm_addr_t *addr, comm_callback_t cb) {
	char* serialized = stringify_char(character);
	send_data_async((void*)serialized, strlen(serialized), addr, cb);
}
