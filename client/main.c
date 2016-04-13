#include <stdio.h>

#include "comm.addr.h"

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
    
    comm_addr_t addr;
    
    if ( address_from_url("fd://1", &addr) ) {
        
        send_int_async(3, &addr, &response_handler);
        printf("Chau");
    }
    
    
    
}