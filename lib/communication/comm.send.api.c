#include "./headers/comm.send.api.h"

#include <stdlib.h>
#include <unistd.h>

#include "../../utils.h"

/**
 * 
 *  SEND API
 * 
 * 
 **/

void send_data(void *data, unsigned int size, comm_addr_t *addr, comm_error_t *error) {

}

void send_data_async(void * data, unsigned int size, comm_addr_t *addr, comm_callback_t cb) {
	comm_error_t *err = malloc(sizeof(comm_error_t));
	err->code = 0;
	err->msg = "Operacion Exitosa";

	sleep(2);

	cb(err, addr, (char*)data);
}

