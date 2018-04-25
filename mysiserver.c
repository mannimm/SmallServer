#include "siserver.h"
#include "csapp.h"


struct VariableStore {
	char variableName[VAR_MAX];
	char value[VALUE_MAX];
	struct VariableStore * next;
};

typedef struct VariableStore VariableNode;

VariableNode * varhead;

void addVariable(char* variableName, char* value) {
	VariableNode * p = varhead;
	while (p->next != NULL) {
		if (strcmp(p->next->variableName, variableName) == 0) {
			break;
		}
		p = p->next;
	}
	if (p->next == NULL) {
		VariableNode * newNode = (VariableNode *) malloc(sizeof(VariableNode));
		strcpy(newNode->variableName, variableName);
		strcpy(newNode->value, value);
		newNode->next = NULL;
		p->next = newNode;
	} else {
		strcpy(p->next->value, value);
	}
}
int getVariable(char* variableName, char * content) {
	VariableNode * p = varhead;
	while (p->next != NULL) {
		if (strcmp(p->next->variableName, variableName) == 0) {
			break;
		}
		p = p->next;
	}
	if (p->next == NULL) {
		return ERROR;
	} else {
		strcpy(content, p->next->value);
		return SUCCESS;
	}
}

int delVariable(char* variableName) {
	VariableNode * p = varhead;
	while (p->next != NULL) {
		if (strcmp(p->next->variableName, variableName) == 0) {
			break;
		}
		p = p->next;
	}
	if (p->next == NULL) {
		return ERROR;
	} else {
		VariableNode * del = p->next;
		p->next = p->next->next;
		free(del);
		return SUCCESS;
	}
}

/**
 * set variable from client
 */
int set(int secretKey, int sock) {
	char variableName[VAR_MAX];
	char* buffer;
	unsigned int value_size;

	if (secretKey != key) {
		status = ERROR;
	} else {
		status = SUCCESS;
	}
	//status = htonl (status);
	write_n(sock, (char *) &status, sizeof(int8_t));
	write_n(sock, (char*) server_padding, strlen(server_padding));


	if ( status == SUCCESS) {
		read_n (sock, variableName, 15);
		read_n (sock, (char *) &value_size, sizeof(unsigned short));

		buffer = (char *) malloc (value_size);
		memset(buffer, 0, value_size);
		
		read_n (sock, buffer, value_size);


		printf("variableName = %s", variableName);
		printf (" : %s\n", buffer);
		addVariable(variableName, buffer);

	}

	free(buffer);
	return status;
}

/**
 * pass the value of a variable to client
 */
int get(int secretKey, int sock) {
	char variableName[VAR_MAX];
	unsigned short size;
	
	if (secretKey != key) {
		status = ERROR;
		return ERROR;
	}
	read_n(sock, variableName, VAR_MAX);
	printf("variableName = %s\n", variableName);
	char value[VALUE_MAX];
	memset(value, 0, VALUE_MAX);
	int ret = getVariable(variableName, value);
	if (ret == ERROR) { // file do not exsit
		status = ERROR;
	} else {
		status = SUCCESS;
	}

	write_n(sock, (char*) &status, sizeof(int8_t));
	write_n(sock, (char*) server_padding, 3);
	if (status == SUCCESS){
		size = strlen(value);
		write_n(sock, (char*) &size, sizeof(unsigned short));
		write_n(sock, value, size);
	}

	return status;	
}



/**
 * delete file
 */
/*
int removefile(HEADER header, int sock) {
	char variableName[VAR_MAX];
	read_n(sock, variableName, VAR_MAX);
	printf("variableName = %s\n", variableName);
	if (header.secretKey != key) {
		unsigned int status = ERROR;
		write_n(sock, (char*) &status, sizeof(status));
		return ERROR;
	}
	unsigned int status;
	if (delVariable(variableName) == SUCCESS) { // delete file successfully
		status = SUCCESS;
	} else {
		status = ERROR;
	}
	write_n(sock, (char*) &status, sizeof(status));
	return status;
}

/**
 * list cuurent file list
 * send to client
 */
/*
int list(HEADER header, int sock) {
	printf("variableName = NONE\n");
	unsigned int status;
	if (header.secretKey != key) {
		status = ERROR;
		write_n(sock, (char*) &status, sizeof(status));
		return ERROR;
	}
	char buffer[10000];
	memset(buffer, 0, 10000);
	VariableNode * p = varhead;
	while (p->next != NULL) {
		strcat(buffer, p->next->variableName);
		strcat(buffer, "\n");
		p = p->next;
	}

	status = SUCCESS;
	write_n(sock, (char*) &status, sizeof(status));
	unsigned int size = strlen(buffer) + 1;
	write_n(sock, (char*) &size, sizeof(size));
	write_n(sock, buffer, size);
	return 0;
}


*/
void message_echo(int socket_fd) {
	int secretKey;
	short type, pad;

	read_n(socket_fd, &secretKey, sizeof(int));
	read_n(socket_fd, &type, sizeof(short));
	read_n(socket_fd, &pad, sizeof(short));
	secretKey 	= ntohl (secretKey);
	type 		= ntohs (type);

	printf("Secret Key = %d\nRequest Type = %s\n", secretKey,
			getConnectionType(type));
	if (type == SET) {
		if (set(secretKey, socket_fd) != -1) {
			printf("Operation Status = success\n--------------------------\n");
		} else {
			printf("Operation Status = error\n--------------------------\n");
		}
	} else if (type == GET) {
		if (get(secretKey, socket_fd) != -1) {
			printf("Operation Status = success\n--------------------------\n");
		} else {
			printf("Operation Status = error\n--------------------------\n");
		}
	} else if (type == DIGEST) {
		if (set(secretKey, socket_fd) != -1) {
			printf("Operation Status = success\n--------------------------\n");
		} else {
			printf("Operation Status = error\n--------------------------\n");
		}
	} else if (type == RUN) {
		if (set(secretKey, socket_fd) != -1) {
			printf("Operation Status = success\n--------------------------\n");
		} else {
			printf("Operation Status = error\n--------------------------\n");
		}
	}
}

int main(int argc, char **argv) {
	setvbuf(stdout, NULL, _IONBF, 0);
	if (argc != 3) {
		fprintf(stderr, "usage: %s <port> <secretKey>\n", argv[0]);
		return -1;
	}
	VariableNode variableNode;
	variableNode.next = NULL;
	varhead = &variableNode;
	int port = atoi(argv[1]);
	key = atoi(argv[2]);
	int listenfd, connfd;
	socklen_t clientlen;
	struct sockaddr_in clientaddr;
	listenfd = Open_listenfd(port);
	while (1) {
		clientlen = sizeof(clientaddr);
		connfd = Accept(listenfd, (SA *) &clientaddr, &clientlen);
		message_echo(connfd);
	}
}

