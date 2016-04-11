#include "comm.send.api.h"

#include <stdlib.h>
#include <unistd.h>

/**
 * 
 *  SEND API
 * 
 * 
 **/

void send_message(char *message, comm_addr_t *addr, comm_error_t *error) {
    
}

void send_int(int number, comm_addr_t *addr, comm_error_t *error) {
    
}

void send_double(int number, comm_addr_t *addr, comm_error_t *error) {
    
}

void send_char(char character, comm_addr_t *addr, comm_error_t *error) {
    
}

void send_data(void *data, unsigned int size, comm_addr_t *addr, comm_error_t *error) {
    
}





void send_message_async(char *message, comm_addr_t *addr, comm_callback_t cb) {
    
}

void send_int_async(int number, comm_addr_t *addr, comm_callback_t cb) {
    
    comm_error_t *err = malloc(sizeof(comm_error_t));
    err->code = 0;
    err->msg = "Operacion Exitosa";
    
    sleep(2);
    
    cb(err, addr, "Hola");
    
}

void send_double_async(int number, comm_addr_t *addr, comm_callback_t cb) {
    
}

void send_char_async(char character, comm_addr_t *addr, comm_callback_t cb) {
    
}

void send_data_async(void * data, unsigned int size, comm_addr_t *addr, comm_callback_t cb) {
    
}

