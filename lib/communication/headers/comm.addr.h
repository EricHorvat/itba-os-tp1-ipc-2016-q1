#ifndef __COMMUNICATION_ADDRESS_API__
#define __COMMUNICATION_ADDRESS_API__

#include "comm.errors.h"

typedef struct {
    char *url;
    char *protocol;
//    char *user;
//    char *pwd;
    char *host;
//    unsigned int port;
    unsigned char valid;
    unsigned char open;
} comm_addr_t;

unsigned char address_from_url(char *url, comm_addr_t *address);

typedef void (*comm_callback_t)(comm_error_t *err, comm_addr_t *origin, char *response);


comm_error_code_t comm_open(comm_addr_t *address);
#include "comm.send.api.h"
#include "comm.receive.api.h"
comm_error_code_t comm_close(comm_addr_t *address);


/**
 * 
 * "fd://1"
 * "socket://martin@martin:3000"
 * "pipe://30"
 * 
 **/


#endif

