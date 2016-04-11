#include "comm.addr.h"

unsigned char address_from_url(char *url, comm_addr_t *address) {
    
    address->host = "1";
    address->protocol = "fd";
    address->url = url;
    
}
