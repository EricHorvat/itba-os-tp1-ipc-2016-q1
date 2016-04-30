#include <comm.connection.api.h>
#include <comm.addr.api.h>
#include <comm.fifo.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <file_utils.h>

#include <utils.h>

#define INVALID_ADDRESS 2
#define SERVER_OFFLINE 3
#define SERVER_BUSY 4

comm_error_code_t connection_open(connection_t *conn) {

	int fd;

	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ERROR("cant create socket");
		return 1;
	} 
	
	server = gethostbyname(/**OJOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOO*/"127.0.0.1");
	if (server == NULL) {
		ERROR("no such host");
		return 3;
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);

	// HARDCODE ALERT
	serv_addr.sin_port = 9005/*OJOOOOOOOOOOOOOOOOOOOOOOOO*/;

	if (connect(fd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)  {
		ERROR("cant connect to socket");
		return 2;
	}

	int url_len = strlen(conn->client_addr->url);
	
	write_one_by_one(fd,conn->client_addr->url,url_len);
	
	SUCCESS("soy el #1");


	conn->req_fd = fd;
	conn->res_fd = fd;
	conn->state = CONNECTION_STATE_OPEN;
	conn->sense = COMMUNICATION_CLIENT_SERVER;

	return 0;

}

comm_error_code_t connection_close(connection_t *conn) {

	
	conn->state = CONNECTION_STATE_CLOSED;

	return 0;

}