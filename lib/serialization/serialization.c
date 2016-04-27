#include <serialization.h>

#include <utils.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <json.h>


// Stringify

const char* stringify_int(int i) {

	json_object *json_object_object = json_object_new_object();
	json_object *json_object_int = json_object_new_int(i);
	json_object *json_object_string = json_object_new_string("int");

	json_object_object_add(json_object_object, "kind", json_object_string);
	json_object_object_add(json_object_object, "value", json_object_int);

	return json_object_to_json_string(json_object_object);
}

const char* stringify_double(double i) {
	json_object *json_object_object = json_object_new_object();
	json_object *json_object_double = json_object_new_double(i);
	json_object *json_object_string = json_object_new_string("double");

	json_object_object_add(json_object_object, "kind", json_object_string);
	json_object_object_add(json_object_object, "value", json_object_double);

	return json_object_to_json_string(json_object_object);
}

const char* stringify_string(char* s) {
	json_object *json_object_object = json_object_new_object();
	json_object *json_object_string = json_object_new_string(s);
	json_object *json_object_string_kind = json_object_new_string("string");

	json_object_object_add(json_object_object, "kind", json_object_string_kind);
	json_object_object_add(json_object_object, "value", json_object_string);

	return json_object_to_json_string(json_object_object);
}

const char* stringify_data(void* data, size_t size) {
	json_object *json_object_object = json_object_new_object();
	json_object *json_object_string = json_object_new_string((char*)data);
	json_object *json_object_string_kind = json_object_new_string("data");

	json_object_object_add(json_object_object, "kind", json_object_string_kind);
	json_object_object_add(json_object_object, "value", json_object_string);

	return json_object_to_json_string(json_object_object);
}

const char* stringify_command_get(command_get_t *cmd) {
	json_object *json_object_object = json_object_new_object();
	json_object *json_object_string = json_object_new_string(cmd->path);
	json_object *json_object_string_kind = json_object_new_string("command.get");

	json_object_object_add(json_object_object, "kind", json_object_string_kind);
	json_object_object_add(json_object_object, "path", json_object_string);

	return json_object_to_json_string(json_object_object);
}

const char* stringify_command_post(command_post_t *cmd) {
	json_object *json_object_object = json_object_new_object();
	json_object *json_object_string = json_object_new_string(cmd->data);
	json_object *json_object_string_kind = json_object_new_string("command.post");
	json_object *json_object_string_dest = json_object_new_string(cmd->dest);

	json_object_object_add(json_object_object, "kind", json_object_string_kind);
	json_object_object_add(json_object_object, "data", json_object_string);
	json_object_object_add(json_object_object, "dest", json_object_string_dest);

	return json_object_to_json_string(json_object_object);
}

// Parse

parse_result_t *parse_encoded(const char *json) {

	const char *kind;
	json_object *main_object, *aux_object;
	const char *str_value;
	command_get_t *get_cmd;
	command_post_t *post_cmd;

	parse_result_t *result = NEW(parse_result_t);

	main_object = json_tokener_parse(json);

	json_object_object_get_ex(main_object, "kind", &aux_object);

	kind = json_object_get_string(aux_object);

	result->kind = (char*)malloc(strlen(kind)+1);
	strcpy(result->kind, kind);

	if (strcmp(kind, "int") == 0) {

		json_object_object_get_ex(main_object, "value", &aux_object);
		result->data.i = json_object_get_int(aux_object);
		return result;

	} else if (strcmp(kind, "string") == 0) {

		json_object_object_get_ex(main_object, "value", &aux_object);
		str_value = json_object_get_string(aux_object);
		result->data.str = (char*)malloc(strlen(str_value)+1);
		strcpy(result->data.str, str_value);
		return result;

	} else if (strcmp(kind, "double") == 0) {

		json_object_object_get_ex(main_object, "value", &aux_object);
		result->data.d = json_object_get_double(aux_object);
		return result;
		
	} else if (strcmp(kind, "data") == 0) {

		json_object_object_get_ex(main_object, "value", &aux_object);
		str_value = json_object_get_string(aux_object);
		result->data.data = (char*)malloc(strlen(str_value)+1);
		memcpy(result->data.data, str_value, strlen(str_value));
		return result;
		
	} else if (strcmp(kind, "command.post") == 0) {

		post_cmd = NEW(command_post_t);

		json_object_object_get_ex(main_object, "data", &aux_object);
		str_value = json_object_get_string(aux_object);
		post_cmd->data = (char*)malloc(strlen(str_value)+1);
		strcpy(post_cmd->data, str_value);

		json_object_object_get_ex(main_object, "dest", &aux_object);
		str_value = json_object_get_string(aux_object);
		post_cmd->dest = (char*)malloc(strlen(str_value)+1);
		strcpy(post_cmd->dest, str_value);
		
		result->data.post_cmd = post_cmd;

		return result;
		
	} else if (strcmp(kind, "command.get") == 0) {
		
		json_object_object_get_ex(main_object, "path", &aux_object);
		str_value = json_object_get_string(aux_object);
		get_cmd->path = (char*)malloc(strlen(str_value)+1);
		strcpy(get_cmd->path, str_value);
		
		result->data.get_cmd = get_cmd;

		return result;

	} else {
		fprintf(stderr, ANSI_COLOR_RED"Unknown kind\n"ANSI_COLOR_RESET);
	}
	fprintf(stderr, ANSI_COLOR_RED"leaving null\n"ANSI_COLOR_RESET);

	return result;
}

// Sync

void send_string(char *string, connection_t *conn, comm_sense_t sense, comm_error_t *error) {
	const char* serialized = stringify_string(string);
	comm_send_data((void*)serialized, strlen(serialized), conn, sense, error);
}

void send_int(int number, connection_t *conn, comm_sense_t sense, comm_error_t *error) {
	printf("sending int %d\n", number);
	const char* serialized = stringify_int(number);
	comm_send_data((void*)serialized, strlen(serialized), conn, sense, error);
}

void send_double(double number, connection_t *conn, comm_sense_t sense, comm_error_t *error) {
	const char* serialized = stringify_double(number);
	comm_send_data((void*)serialized, strlen(serialized), conn, sense, error);
}

void send_data(void *data, size_t size, connection_t *conn, comm_sense_t sense, comm_error_t *error) {
	const char* serialized = stringify_data(data, size);
	comm_send_data((void*)serialized, strlen(serialized), conn, sense, error);
}

// Async

void send_string_async(char *string, connection_t *conn, comm_sense_t sense, comm_callback_t cb) {
	const char* serialized = stringify_string(string);
	comm_send_data_async((void*)serialized, strlen(serialized), conn, sense, cb);
}

void send_int_async(int number, connection_t *conn, comm_sense_t sense, comm_callback_t cb) {
	const char* serialized = stringify_int(number);
	comm_send_data_async((void*)serialized, strlen(serialized), conn, sense, cb);
}

void send_double_async(double number, connection_t *conn, comm_sense_t sense, comm_callback_t cb) {
	const char* serialized = stringify_double(number);
	comm_send_data_async((void*)serialized, strlen(serialized), conn, sense, cb);
}

void send_data_async(void *data, size_t size, connection_t *conn, comm_sense_t sense, comm_callback_t cb) {
	const char* serialized = stringify_data(data, size);
	comm_send_data_async((void*)serialized, strlen(serialized), conn, sense, cb);
}


// Commands

// Sync

void send_cmd_get(command_get_t *cmd, connection_t *conn, comm_sense_t sense, comm_error_t *error) {
	const char* serialized = stringify_command_get(cmd);
	comm_send_data((void*)serialized, strlen(serialized), conn, sense, error);
}

void send_cmd_post(command_post_t *cmd, connection_t *conn, comm_sense_t sense, comm_error_t *error) {
	const char* serialized = stringify_command_post(cmd);
	comm_send_data((void*)serialized, strlen(serialized), conn, sense, error);
}

// Async

void send_cmd_get_async(command_get_t *cmd, connection_t *conn, comm_sense_t sense, comm_callback_t cb) {
	const char* serialized = stringify_command_get(cmd);
	comm_send_data_async((void*)serialized, strlen(serialized), conn, sense, cb);
}

void send_cmd_post_async(command_post_t *cmd, connection_t *conn, comm_sense_t sense, comm_callback_t cb) {
	const char* serialized = stringify_command_post(cmd);
	comm_send_data_async((void*)serialized, strlen(serialized), conn, sense, cb);
}
