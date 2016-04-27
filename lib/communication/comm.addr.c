#include <comm.addr.h>

#include <utils.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

comm_addr_error_t address_from_url(char *url, comm_addr_t *address) {

	fprintf(stderr, "addr from url\n");

	// Read protocol first
	unsigned int index = 0;
	char *start, *offset;
	start = offset = url;

	if (address == nil) {
		fprintf(stderr, "addr is nil\n");
		return -1;
	}

	address->valid = yes;
	address->url = url;

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
	fprintf(stderr, "before malloc\n");
	address->protocol = (char*)malloc(index);
	strncpy(address->protocol, start, index);
	fprintf(stderr, ANSI_COLOR_YELLOW"addr::protocol = %s\n"ANSI_COLOR_RESET, address->protocol);

	// Read host
	start = offset;
	index = 0;

	while (*offset != COLON && *offset != NEWLINE && *offset != EOF && *offset != ZERO) {
		++offset;
		++index;
	}
	
	address->host = (char*)malloc(index);
	strncpy(address->host, start, index);
	printf(ANSI_COLOR_YELLOW"addr::host = %s\n"ANSI_COLOR_RESET, address->host);

	if (*offset == NEWLINE || *offset == EOF || *offset == ZERO) {
		return 0;
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
	
	address->port = atoi(start);
	
	return 0;
}
