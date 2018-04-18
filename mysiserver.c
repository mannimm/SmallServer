#include "cloud.h"
#define FILELENGTH 100 * 1024

struct FileStore {
	char fileName[FILE_NAME_LENGTH];
	char fileContent[FILELENGTH];
	struct FileStore * next;
};

typedef struct FileStore FileNode;

FileNode * filehead;
void addFile(char* fileName, char* fileContent) {
	FileNode * p = filehead;
	while (p->next != NULL) {
		if (strcmp(p->next->fileName, fileName) == 0) {
			break;
		}
		p = p->next;
	}
	if (p->next == NULL) {
		FileNode * newNode = (FileNode *) malloc(sizeof(FileNode));
		strcpy(newNode->fileName, fileName);
		strcpy(newNode->fileContent, fileContent);
		newNode->next = NULL;
		p->next = newNode;
	} else {
		strcpy(p->next->fileContent, fileContent);
	}
}
int getFile(char* fileName, char * content) {
	FileNode * p = filehead;
	while (p->next != NULL) {
		if (strcmp(p->next->fileName, fileName) == 0) {
			break;
		}
		p = p->next;
	}
	if (p->next == NULL) {
		return ERROR;
	} else {
		strcpy(content, p->next->fileContent);
		return SUCCESS;
	}
}

int delFile(char* fileName) {
	FileNode * p = filehead;
	while (p->next != NULL) {
		if (strcmp(p->next->fileName, fileName) == 0) {
			break;
		}
		p = p->next;
	}
	if (p->next == NULL) {
		return ERROR;
	} else {
		FileNode * del = p->next;
		p->next = p->next->next;
		free(del);
		return SUCCESS;
	}
}

/**
 * recieve file from client
 */
int put(HEADER header, int sock) {
	char fileName[FILE_NAME_LENGTH];
	read_n(sock, fileName, FILE_NAME_LENGTH);
	printf("Filename = %s\n", fileName);
	unsigned int fileSize;
	read_n(sock, (char*) &fileSize, 4);
	char* buffer = (char*) malloc(sizeof(char) * fileSize);
	memset(buffer, 0, fileSize);
	read_n(sock, buffer, fileSize);
	addFile(fileName, buffer);
	unsigned int status;
	if (header.secretKey != key) {
		status = ERROR;
	} else {
		status = SUCCESS;
	}
	write_n(sock, (char*) &status, sizeof(status));
	free(buffer);
	return status;
}

/**
 * send file to client
 */
int get(HEADER header, int sock) {
	char fileName[FILE_NAME_LENGTH];
	read_n(sock, fileName, FILE_NAME_LENGTH);
	printf("Filename = %s\n", fileName);
	if (header.secretKey != key) {
		unsigned int status = ERROR;
		write_n(sock, (char*) &status, sizeof(status));
		return ERROR;
	}
	char fileContent[FILELENGTH];
	memset(fileContent, 0, FILELENGTH);
	int ret = getFile(fileName, fileContent);
	if (ret == ERROR) { // file do not exsit
		unsigned int status = ERROR;
		write_n(sock, (char*) &status, sizeof(status));
		return ERROR;
	} else {
		unsigned int returnStatus = SUCCESS;
		write_n(sock, (char*) &returnStatus, sizeof(returnStatus));
		unsigned int size = strlen(fileContent) + 1;
		write_n(sock, (char*) &size, sizeof(size));
		write_n(sock, fileContent, size);
		return SUCCESS;
	}
}

/**
 * delete file
 */
int removefile(HEADER header, int sock) {
	char fileName[FILE_NAME_LENGTH];
	read_n(sock, fileName, FILE_NAME_LENGTH);
	printf("Filename = %s\n", fileName);
	if (header.secretKey != key) {
		unsigned int status = ERROR;
		write_n(sock, (char*) &status, sizeof(status));
		return ERROR;
	}
	unsigned int status;
	if (delFile(fileName) == SUCCESS) { // delete file successfully
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
int list(HEADER header, int sock) {
	printf("Filename = NONE\n");
	unsigned int status;
	if (header.secretKey != key) {
		status = ERROR;
		write_n(sock, (char*) &status, sizeof(status));
		return ERROR;
	}
	char buffer[10000];
	memset(buffer, 0, 10000);
	FileNode * p = filehead;
	while (p->next != NULL) {
		strcat(buffer, p->next->fileName);
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

void message_echo(int socket_fd) {
	HEADER header;

	memset(&header, 0, sizeof(header));
	read_n(socket_fd, (char*) &header, sizeof(header));

	printf("Secret Key = %d\nRequest Type = %s\n", header.secretKey,
			getStringType(header.type));
	if (header.type == LIST) {
		if (list(header, socket_fd) != -1) {
			printf("Operation Status = success\n--------------------------\n");
		} else {
			printf("Operation Status = error\n--------------------------\n");
		}
	} else if (header.type == PUT) {
		if (put(header, socket_fd) != -1) {
			printf("Operation Status = success\n--------------------------\n");
		} else {
			printf("Operation Status = error\n--------------------------\n");
		}
	} else if (header.type == GET) {
		if (get(header, socket_fd) != -1) {
			printf("Operation Status = success\n--------------------------\n");
		} else {
			printf("Operation Status = error\n--------------------------\n");
		}
	} else if (header.type == DEL) {
		if (removefile(header, socket_fd) != -1) {
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
	FileNode filenode;
	filenode.next = NULL;
	filehead = &filenode;
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

