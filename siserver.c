#include "siserver.h"

char * getConnectionType(int type) {
	if (type == GET) {
		return "get";
	} else if (type == SET) {
		return "set";
	} else if (type == DIGEST) {
		return "del";
	} else {
		return "list";
	}
}

/*
 This function writes "n" characters to the server.
 */

int write_n(int fd, void *ptr, int n_bytes) {
	int n_left, n_written;
	n_left = n_bytes;
	while (n_left > 0) {
		n_written = write(fd, ptr, n_left);
		if (n_written <= 0) {
			return (n_written);
		}
		n_left = n_left - n_written;
		ptr = ptr + n_written;
	}
	return (n_bytes - n_left);
}

/*
 This function reads "n" characters from the server.
 */

int read_n(int fd, void *ptr, int n_bytes) {
	int n_left, n_read;
	n_left = n_bytes;
	while (n_left > 0) {
		n_read = read(fd, ptr, n_left);
		if (n_read < 0) {
			return (n_read);
		} else if (n_read == 0) {
			break;
		}
		n_left = n_left - n_read;
		ptr = ptr + n_read;
	}
	return (n_bytes - n_left);
}



int smallSet(char *MachineName, int Port, int SecretKey,
			char *VariableName, char* Value, int dataLength) {

	int socket_fd;	
	short pad = 0;
	char *host = (char *) calloc(1, 40);		

    strncpy (host, MachineName, sizeof(host));		
    short int Type = SET;
    char status, server_padding[3];

    char *variableName 	= (char *) calloc(1, VAR_MAX);	// Allocate space for variable name.
    char *value 		= (char *) calloc(1, VALUE_MAX);	// Allocate space for value.

    strncpy (variableName, VariableName, sizeof(variableName));			// Read variable name from argument list.
    strncpy (value, Value, sizeof(value));

   	if ( (socket_fd = Open_clientfd(host, Port)) < 0 )	// Open connection to provided Host and Port.
		return(-1);

	SecretKey = htonl(SecretKey);
	write_n(socket_fd, (int*) &SecretKey, sizeof(int));
	
	Type = htons(Type);
	write_n(socket_fd, &Type, sizeof(short));
	write_n(socket_fd, &pad, sizeof(pad));
	write_n(socket_fd, (char* ) variableName, VAR_MAX);		// Send VariableName over to the server.

	short value_size = strlen(Value);
	short value_size_neto = htons(value_size);

	write_n(socket_fd, &value_size_neto, sizeof(short));
	write_n(socket_fd, (char *) &value, value_size);			// Send Value over to the server.

	read_n(socket_fd, (char*) &status, sizeof(status));
	Close(socket_fd);
	if (status == SUCCESS) {
		//printf("Success\n");
		return SUCCESS;
	} else if (status == ERROR) {
		printf("Error\n");
		return ERROR;
	}

   	free(variableName);
   	free(value);
	return -1;
	
}
	

int smallGet(char *MachineName, int Port, int SecretKey,
			char *VariableName) {

	int socket_fd;
	short value_size;
	short int pad = 0, Type = GET;
	char *host = (char *) calloc(1, 40);		
    char status, server_padding[3];

    char *variableName 	= (char *) calloc(1, VAR_MAX);	// Allocate space for variable name.
    char *value 		= (char *) calloc(1, VALUE_MAX);	// Allocate space for value.

    strncpy (variableName, VariableName, sizeof(variableName));			// Read variable name from argument list.
    strncpy (host, MachineName, sizeof(host));

   	if ( (socket_fd = Open_clientfd(host, Port)) < 0 )	// Open connection to provided Host and Port.
		return(-1);

	SecretKey = htonl(SecretKey);
	write_n(socket_fd, &SecretKey, sizeof(int));
	
	Type = htons(Type);
	write_n(socket_fd, &Type, sizeof(short));
	write_n(socket_fd, &pad, sizeof(short));
	write_n(socket_fd, (char* ) variableName, VAR_MAX);		// Send VariableName over to the server.
	
	read_n(socket_fd, &status, sizeof(char));
	read_n(socket_fd, &server_padding, sizeof(server_padding));
	

	if ( status == SUCCESS) {
		read_n(socket_fd, &value_size, sizeof(short));
		value_size = ntohs (value_size);
		read_n(socket_fd, (char*) &value, value_size);
		printf("Success\n");
	
	} else if (status == ERROR) {
		printf("Error\n");
		
	}


	Close(socket_fd);

   	free(variableName);
   	free(value);
	return status;	

}
	


 // Error-handling functions

void unix_error(char *msg) /* Unix-style error */
{
	fprintf(stderr, "%s: %s\n", msg, strerror(errno));
	exit(0);
}
/* $end unixerror */

void posix_error(int code, char *msg) /* Posix-style error */
{
	fprintf(stderr, "%s: %s\n", msg, strerror(code));
	exit(0);
}

void dns_error(char *msg) /* DNS-style error */
{
	fprintf(stderr, "%s: DNS error %d\n", msg, h_errno);
	exit(0);
}

void app_error(char *msg) /* Application error */
{
	fprintf(stderr, "%s\n", msg);
	exit(0);
}
/* $end errorfuns */

/********************************
 * Client/server helper functions
 ********************************/
int open_clientfd(char *hostname, int port) {
	int clientfd;
	struct hostent *hp;
	struct sockaddr_in serveraddr;

	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1; /* Check errno for cause of error */

	/* Fill in the server's IP address and port */
	if ((hp = gethostbyname(hostname)) == NULL)
		return -2; /* Check h_errno for cause of error */
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	bcopy((char *) hp->h_addr_list[0], (char *) &serveraddr.sin_addr.s_addr,
			hp->h_length);
	serveraddr.sin_port = htons(port);

	/* Establish a connection with the server */
	if (connect(clientfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
		return -1;
	return clientfd;
}
/* $end open_clientfd */

/*
 * open_listenfd - open and return a listening socket on port
 *     Returns -1 and sets errno on Unix error.
 */
int open_listenfd(int port) {
	int listenfd, optval = 1;
	struct sockaddr_in serveraddr;

	
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		return -1;

	if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *) &optval,
			sizeof(int)) < 0)
		return -1;

	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short) port);
	if (bind(listenfd, (SA *) &serveraddr, sizeof(serveraddr)) < 0)
		return -1;

	/* Make it a listening socket ready to accept connection requests */
	if (listen(listenfd, LISTENQ) < 0)
		return -1;
	return listenfd;
}
/* $end open_listenfd */

/******************************************
 * Wrappers for the client/server helper routines
 ******************************************/
int Open_clientfd(char *hostname, int port) {
	int rc;

	if ((rc = open_clientfd(hostname, port)) < 0) {
		if (rc == -1)
			unix_error("Open_clientfd Unix error");
		else
			dns_error("Open_clientfd DNS error");
	}
	return rc;
}

int Open_listenfd(int port) {
	int rc;

	if ((rc = open_listenfd(port)) < 0)
		unix_error("Open_listenfd error");
	return rc;
}
void Close(int fd) {
	int rc;

	if ((rc = close(fd)) < 0)
		unix_error("Close error");
}
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen) {
	int rc;

	if ((rc = accept(s, addr, addrlen)) < 0)
		unix_error("Accept error");
	return rc;
}
