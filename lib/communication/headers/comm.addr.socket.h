#ifndef __COMMUNICATION_ADDRESS_API__
#define __COMMUNICATION_ADDRESS_API__

#include "comm.addr.h"

typedef struct {
    comm_addr_t *addr;
    unsigned int port;
} comm_addr_socket_t;

unsigned char address_socket_from_url(char *url, comm_addr_socket_t *address);


#endif

