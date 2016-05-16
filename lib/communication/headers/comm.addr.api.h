#ifndef __COMMUNICATION_ADDRESS_API_H__
#define __COMMUNICATION_ADDRESS_API_H__

#include <comm.errors.h>
#include <utils.h>

#ifdef __FIFO__
#include <comm.addr.fifo.h>
#else
#include <comm.addr.socket.h>
#endif

typedef struct {
	char*              url;
	char*              protocol;
	char*              host;
	boolean               valid;
	comm_addr_extra_t* extra;
} comm_addr_t;

/**
 * receives an allocated comm_addr_t and fills it
 * @param  url     url to buil address from
 * @param  address allocated comm_addr_t
 * @return         0 if url was ok. other number otherwise
 */
comm_addr_error_t address_from_url(char* url, comm_addr_t* address);

#endif
