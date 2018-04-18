#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/file.h>
#include <errno.h>
#include <unistd.h>


#define FILE_NAME_LENGTH 80

#define SET 0
#define GET	1



#define SUCCESS 0
#define ERROR -1

#define VALUE_MAX 100000
#define VAR_MAX 15

unsigned int key;

#define LISTENQ  1024  /* Second argument to listen() */

typedef struct sockaddr SA;
typedef struct {
	unsigned int secretKey;
	unsigned int type;
} HEADER;

char * getStringType(int type);
int smallSet(char *MachineName, int Port, int SecretKey, char *variableName, char* value, int dataLength);
int smallGet(char *MachineName, int Port, int SecretKey, char *variableName);


int write_n(int fd, char *ptr, int n_bytes);
int read_n(int fd, char *ptr, int n_bytes);

/* Our own error-handling functions */
void unix_error(char *msg);
void posix_error(int code, char *msg);
void dns_error(char *msg);
void app_error(char *msg);

/* Client/server helper functions */
int open_clientfd(char *hostname, int portno);
int open_listenfd(int portno);

/* Wrappers for client/server helper functions */
int Open_clientfd(char *hostname, int port);
int Open_listenfd(int port);
void Close(int fd);
int Accept(int s, struct sockaddr *addr, socklen_t *addrlen);
