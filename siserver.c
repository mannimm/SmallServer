//my version 5
#include "siserver.h"
#include "csapp.h"

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

    strncpy (host, MachineName, (int) sizeof(host));		
    short int Type = SET;
    char status;

    char *variableName 	= malloc (VAR_MAX+1);	// Allocate space for variable name.
    //memset (variableName, 0, VAR_MAX);
    char *value 		= malloc (VALUE_MAX+1);	// Allocate space for value.
    //memset (value, 0, VALUE_MAX);
    strncpy (variableName, VariableName, VAR_MAX);			// Read variable name from argument list.
    strncpy (value, Value, VALUE_MAX);

   	if ( (socket_fd = Open_clientfd(MachineName, Port)) < 0 )	// Open connection to provided Host and Port.
		return(-1);

	SecretKey = htonl(SecretKey);
	write_n(socket_fd, (int*) &SecretKey, sizeof(int));
	
	Type = htons(Type);
	write_n(socket_fd, &Type, sizeof(short));
	write_n(socket_fd, &pad, sizeof(pad));

	read_n(socket_fd, &status, sizeof(status));
	//read_n(socket_fd, &server_padding, strlen(server_padding));
	status = ntohl (status);
	printf ("status is: %d\n", status);
	if (status == SUCCESS) {
		printf ("var is 	: %s\n", variableName);								//FOR TEST
		printf ("variable size befor is : %d\n", (int) strlen(variableName));	//FOR TEST
		printf ("value is 	: %s\n", value);									//FOR TEST

		write_n(socket_fd, (char* ) variableName, VAR_MAX);		// Send VariableName over to the server.
		unsigned short value_size = strlen(Value);
		printf ("value size: %d\n", value_size);								//FOR TEST
		unsigned short value_size_neto = htons(value_size);
		write_n(socket_fd, &value_size_neto, sizeof(unsigned short));
		write_n(socket_fd, (char *) value, value_size);
		//return SUCCESS;
	} else if (status == ERROR) {
		printf("Error\n");
		//return ERROR;
	}

	Close(socket_fd);
   	free(variableName);
   	free(value);
	return status;
	
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

    strncpy (variableName, VariableName, (int) sizeof(variableName));			// Read variable name from argument list.
    strncpy (host, MachineName, (int) sizeof(host));

   	if ( (socket_fd = Open_clientfd(MachineName, Port)) < 0 )	// Open connection to provided Host and Port.
		return(-1);

	SecretKey = htonl(SecretKey);
	write_n(socket_fd, &SecretKey, sizeof(int));
	
	Type = htons(Type);
	write_n(socket_fd, &Type, sizeof(short));
	write_n(socket_fd, &pad, sizeof(short));
	
	read_n(socket_fd, &status, sizeof(char));
	read_n(socket_fd, &server_padding, strlen(server_padding));
	

	if ( status == SUCCESS) {
		write_n(socket_fd, (char* ) variableName, VAR_MAX);		// Send VariableName over to the server.
		read_n(socket_fd, &value_size, sizeof(short));
		value_size = ntohs (value_size);
		read_n(socket_fd, (char*) &value, value_size);
		printf("value is: %s\n",value);
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


















/*********************************************************************
 * The Rio package - robust I/O functions
 **********************************************************************/
/*
 * rio_readn - robustly read n bytes (unbuffered)
 */
/* $begin rio_readn */
ssize_t rio_readn(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nread = read(fd, bufp, nleft)) < 0) {
	    if (errno == EINTR) /* Interrupted by sig handler return */
		nread = 0;      /* and call read() again */
	    else
		return -1;      /* errno set by read() */ 
	} 
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}
/* $end rio_readn */

/*
 * rio_writen - robustly write n bytes (unbuffered)
 */
/* $begin rio_writen */
ssize_t rio_writen(int fd, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while (nleft > 0) {
	if ((nwritten = write(fd, bufp, nleft)) <= 0) {
	    if (errno == EINTR)  /* Interrupted by sig handler return */
		nwritten = 0;    /* and call write() again */
	    else
		return -1;       /* errno set by write() */
	}
	nleft -= nwritten;
	bufp += nwritten;
    }
    return n;
}
/* 
 * rio_read - This is a wrapper for the Unix read() function that
 *    transfers min(n, rio_cnt) bytes from an internal buffer to a user
 *    buffer, where n is the number of bytes requested by the user and
 *    rio_cnt is the number of unread bytes in the internal buffer. On
 *    entry, rio_read() refills the internal buffer via a call to
 *    read() if the internal buffer is empty.
 */
/* $begin rio_read */
static ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n)
{
    int cnt;

    while (rp->rio_cnt <= 0) {  /* Refill if buf is empty */
	rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, 
			   sizeof(rp->rio_buf));
	if (rp->rio_cnt < 0) {
	    if (errno != EINTR) /* Interrupted by sig handler return */
		return -1;
	}
	else if (rp->rio_cnt == 0)  /* EOF */
	    return 0;
	else 
	    rp->rio_bufptr = rp->rio_buf; /* Reset buffer ptr */
    }

    /* Copy min(n, rp->rio_cnt) bytes from internal buf to user buf */
    cnt = n;          
    if (rp->rio_cnt < n)   
	cnt = rp->rio_cnt;
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}
/* $end rio_read */

/*
 * rio_readinitb - Associate a descriptor with a read buffer and reset buffer
 */
/* $begin rio_readinitb */
void rio_readinitb(rio_t *rp, int fd) 
{
    rp->rio_fd = fd;  
    rp->rio_cnt = 0;  
    rp->rio_bufptr = rp->rio_buf;
}
/* $end rio_readinitb */

/*
 * rio_readnb - Robustly read n bytes (buffered)
 */
/* $begin rio_readnb */
ssize_t rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;
    
    while (nleft > 0) {
	if ((nread = rio_read(rp, bufp, nleft)) < 0) 
            return -1;          /* errno set by read() */ 
	else if (nread == 0)
	    break;              /* EOF */
	nleft -= nread;
	bufp += nread;
    }
    return (n - nleft);         /* return >= 0 */
}
/* $end rio_readnb */

/* 
 * rio_readlineb - robustly read a text line (buffered)
 */
/* $begin rio_readlineb */
ssize_t rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    int n, rc;
    char c, *bufp = usrbuf;

    for (n = 1; n < maxlen; n++) { 
        if ((rc = rio_read(rp, &c, 1)) == 1) {
	    *bufp++ = c;
	    if (c == '\n') {
                n++;
     		break;
            }
	} else if (rc == 0) {
	    if (n == 1)
		return 0; /* EOF, no data read */
	    else
		break;    /* EOF, some data was read */
	} else
	    return -1;	  /* Error */
    }
    *bufp = 0;
    return n-1;
}
/* $end rio_readlineb */

/**********************************
 * Wrappers for robust I/O routines
 **********************************/
ssize_t Rio_readn(int fd, void *ptr, size_t nbytes) 
{
    ssize_t n;
  
    if ((n = rio_readn(fd, ptr, nbytes)) < 0)
	unix_error("Rio_readn error");
    return n;
}

void Rio_writen(int fd, void *usrbuf, size_t n) 
{
    if (rio_writen(fd, usrbuf, n) != n)
	unix_error("Rio_writen error");
}

void Rio_readinitb(rio_t *rp, int fd)
{
    rio_readinitb(rp, fd);
} 

ssize_t Rio_readnb(rio_t *rp, void *usrbuf, size_t n) 
{
    ssize_t rc;

    if ((rc = rio_readnb(rp, usrbuf, n)) < 0)
	unix_error("Rio_readnb error");
    return rc;
}

ssize_t Rio_readlineb(rio_t *rp, void *usrbuf, size_t maxlen) 
{
    ssize_t rc;

    if ((rc = rio_readlineb(rp, usrbuf, maxlen)) < 0)
	unix_error("Rio_readlineb error");
    return rc;
} 

