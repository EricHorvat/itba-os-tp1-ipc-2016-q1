#include <stdio.h>
#include <stdlib.h>

// #include "../lib/communication/headers/communication.h"
#include "../lib/serialization/headers/serialization.h"
#include "../utils.h"

void response_handler(comm_error_t *err, comm_addr_t *addr, char * response) {

	printf("%s dice: %s\n", addr->url, response);

}


/**
 * 
 *          ./server    ./client
 *          Listening to ...
 * 
 * 
 *          Cliente--        --Server
 *                   \      /
 *          Cliente--->----<---Server
 *                   /      \
 *          Cliente--        --Server
 * 
 * 
 **/

 int main (int argc, char **argv) {

 	comm_addr_t *addr;
 	comm_addr_error_t addr_error;

 	addr = (comm_addr_t*)NEW(comm_addr_t);

 	if ( (addr_error = address_from_url("fd://1:3000", addr)) > 0) {

 		printf("Error %d\n", addr_error);
 		return addr_error;
 	}

 	printf("Address Info:\nProtocol: %s\tHost: %s\tPort:%d\n", addr->protocol, addr->host, addr->port);

 	send_int_async(3, addr, &response_handler);

 }