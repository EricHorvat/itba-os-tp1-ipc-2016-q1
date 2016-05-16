#include <comm.addr.api.h>
#include <comm.addr.socket.h>

#include <utils.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

comm_addr_error_t address_from_url(char* url, comm_addr_t* address) {

	// Read protocol first
	unsigned int index = 0;
	char *       start, *offset;
	start = offset = url;

	if (address == nil) {
		fprintf(stderr, "addr is nil\n");
		return ADDRESS_NULL;
	}

	address->valid = yes;
	address->url   = (char*)malloc(strlen(url) + 1);
	memset(address->url, ZERO, strlen(url) + 1);
	strcpy(address->url, url);

	if (*offset != 's')
		return ADDRESS_INVALID_PROTOCOL;

	while (*offset != COLON && *offset != NEWLINE && *offset != EOF && *offset != ZERO) {
		++offset;
		++index;
	}

	if (*offset == NEWLINE || *offset == EOF || *offset == ZERO) {
		address->valid = no;
		return ADDRESS_MISSING_PROTOCOL_TOKEN;
	}

	++offset;

	if (offset[0] != SLASH || offset[1] != SLASH) {
		address->valid = no;
		return ADDRESS_MISSING_PROTOCOL_TOKEN;
	}
	++offset;
	++offset;
	address->protocol = (char*)malloc(index + 1);
	memset(address->protocol, ZERO, index + 1);
	strncpy(address->protocol, start, index);

	// Read host
	start = offset;
	index = 0;

	while (*offset != COLON && *offset != NEWLINE && *offset != EOF && *offset != ZERO) {
		++offset;
		++index;
	}

	address->host = (char*)malloc(index + 1);
	memset(address->host, ZERO, index + 1);
	strncpy(address->host, start, index);

	if (*offset == NEWLINE || *offset == EOF || *offset == ZERO) {
		return ADDRESS_MISSING_PORT_TOKEN;
	}

	// Read Port
	if (*offset != COLON) {
		address->valid = no;
		return ADDRESS_MISSING_PORT_TOKEN;
	}
	++offset;
	start = offset;
	index = 0;

	while (*offset != ZERO && *offset != NEWLINE && *offset != EOF) {
		if (*offset < '0' || '9' < *offset) {
			address->valid = no;
			return ADDRESS_INVALID_PORT;
		}
		++offset;
	}

	address->extra = NEW(comm_addr_extra_t);

	address->extra->port = atoi(start);

	return ADDRESS_OK;
}
