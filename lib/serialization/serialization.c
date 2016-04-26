#include <serialization.h>

#include <utils.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <json.h>

static const char* stringify_int(int);
static const char* stringify_double(double);
static const char* stringify_string(char*);
static const char* stringify_data(void*, size_t);

// Stringify

static const int BASE_LENGTH = 18;
static const int INT_LENGTH = 3;
static const int FLOAT_LENGTH = 5;
static const int DOUBLE_LENGTH = 6;
static const int CHAR_LENGTH = 4;
static const int STRING_LENGTH = 6;

static const char* stringify_int(int i) {

	json_object *json_object_object = json_object_new_object();
	json_object *json_object_int = json_object_new_int(i);
	json_object *json_object_string = json_object_new_string("int");

	json_object_object_add(json_object_object, "kind", json_object_string);
	json_object_object_add(json_object_object, "value", json_object_int);

	return json_object_to_json_string(json_object_object);
}

static const char* stringify_double(double i) {
	json_object *json_object_object = json_object_new_object();
	json_object *json_object_double = json_object_new_double(i);
	json_object *json_object_string = json_object_new_string("double");

	json_object_object_add(json_object_object, "kind", json_object_string);
	json_object_object_add(json_object_object, "value", json_object_double);

	return json_object_to_json_string(json_object_object);
}

static const char* stringify_string(char* s) {
	json_object *json_object_object = json_object_new_object();
	json_object *json_object_string = json_object_new_string(s);
	json_object *json_object_string_kind = json_object_new_string("string");

	json_object_object_add(json_object_object, "kind", json_object_string_kind);
	json_object_object_add(json_object_object, "value", json_object_string);

	return json_object_to_json_string(json_object_object);
}

static const char* stringify_data(void* data, size_t size) {
	json_object *json_object_object = json_object_new_object();
	json_object *json_object_string = json_object_new_string((char*)data);
	json_object *json_object_string_kind = json_object_new_string("data");

	json_object_object_add(json_object_object, "kind", json_object_string_kind);
	json_object_object_add(json_object_object, "value", json_object_string);

	return json_object_to_json_string(json_object_object);
}

// Sync

void send_string(char *string, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error) {
	const char* serialized = stringify_string(string);
	comm_send_data((void*)serialized, strlen(serialized), origin, endpoint, error);
}

void send_int(int number, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error) {
	printf("sending int %d\n", number);
	const char* serialized = stringify_int(number);
	comm_send_data((void*)serialized, strlen(serialized), origin, endpoint, error);
}

void send_double(double number, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error) {
	const char* serialized = stringify_double(number);
	comm_send_data((void*)serialized, strlen(serialized), origin, endpoint, error);
}

void send_data(void *data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_error_t *error) {
	const char* serialized = stringify_data(data, size);
	comm_send_data((void*)serialized, strlen(serialized), origin, endpoint, error);
}

// Async

void send_string_async(char *string, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb) {
	const char* serialized = stringify_string(string);
	comm_send_data_async((void*)serialized, strlen(serialized), origin, endpoint, cb);
}

void send_int_async(int number, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb) {
	const char* serialized = stringify_int(number);
	comm_send_data_async((void*)serialized, strlen(serialized), origin, endpoint, cb);
}

void send_double_async(double number, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb) {
	const char* serialized = stringify_double(number);
	comm_send_data_async((void*)serialized, strlen(serialized), origin, endpoint, cb);
}

void send_data_async(void *data, size_t size, comm_addr_t *origin, comm_addr_t *endpoint, comm_callback_t cb) {
	const char* serialized = stringify_data(data, size);
	comm_send_data_async((void*)serialized, strlen(serialized), origin, endpoint, cb);
}


