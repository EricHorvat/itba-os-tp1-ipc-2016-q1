#include <comm.connection.api.h>
#include <comm.addr.api.h>
#include <comm.socket.h>
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

comm_error_code_t connection_open(connection_t* conn) {

	int fd;

	struct sockaddr_in serv_addr;
	struct hostent*    server;
	int                url_len;

	if ((fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		ERROR("cant create socket");
		return ERR_SOCKET_NOT_CREATED;
	}

	server = gethostbyname(conn->server_addr->host);
	if (server == NULL) {
		ERROR("no such host");
		return ERR_NO_SUCH_HOST;
	}
	bzero((char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);

	INFO("port: %d", conn->server_addr->extra->port);
	serv_addr.sin_port = conn->server_addr->extra->port;
	INFO("port %d", conn->server_addr->extra->port);
	if (connect(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
		ERROR("cant connect to socket");
		return ERR_SOCKET_NOT_CONNECTED;
	}

	url_len = strlen(conn->client_addr->url);

	write_one_by_one(fd, conn->client_addr->url, url_len);

	conn->req_fd = fd;
	conn->res_fd = fd;
	conn->state  = CONNECTION_STATE_OPEN;
	conn->sense  = COMMUNICATION_CLIENT_SERVER;

	return CONNECTION_OK;
}

comm_error_code_t connection_close(connection_t* conn) {

	conn->state = CONNECTION_STATE_CLOSED;

	return CONNECTION_OK;
}
