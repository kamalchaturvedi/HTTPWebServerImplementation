/*
 * WebServer.c
 *
 *  Created on: Oct 13, 2018
 *      Author: kamalchaturvedi15
 */

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#include<arpa/inet.h>
#include<string.h>
#include<sys/sendfile.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <sys/wait.h>
#include <errno.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#define BUFSIZE 2048

typedef struct {
	char *ext;
	char *mediatype;
} extn;

int checkError(int, char *, int*);
unsigned long fileSize(char *);
void handleClientRequest(int, char[], int, char[], extn[]);
void writeToFile(char*, char*, int);

extn fileExtensions[] = { { "gif", "image/gif" }, { "txt", "text/plain" }, {
		"jpg", "image/jpg" }, { "js", "application/javascript" }, { "css",
		"text/css" }, { "jpeg", "image/jpeg" }, { "png", "image/png" }, { "ico",
		"image/ico" }, { "zip", "image/zip" }, { "gz", "image/gz" }, { "tar",
		"image/tar" }, { "htm", "text/html" }, { "html", "text/html" }, { "php",
		"text/html" }, { "zip", "application/octet-stream" }, { "rar",
		"application/octet-stream" }, { 0, 0 } };

char fileNotFoundResponse[] =
		"HTTP/1.0 404 Not Found\r\n"
				"<html><head><title>404 Not Found</head></title>\r\n"
				"<body><p>404 Not Found: The requested resource could not be found!</p></body></html>\r\n";

char internalServerErrorResponse[] =
		"HTTP/1.0 500 Internal Server Error\r\n"
				"<html><head><title>500 Internal Server Error</head></title><body><p>\r\n"
				"These messages in the exact format as shown above should be sent back to the client if any of the above error occurs. "
				"</p></body></html>\r\n";

char baseResponse[] = "./www";

/*Method to write to file in the case of  POST requests*/
void writeToFile(char* bufferPointer, char* fileResource, int clientSocket) {
	char tempBuffer[2048];
	unsigned long requestedfileSize;

	sprintf(tempBuffer, "<html><body><pre><h1>%s</h1></pre>", bufferPointer);
	requestedfileSize = fileSize(fileResource);
	char *existingContent = malloc(requestedfileSize + 1);
	FILE * fp;
	fp = fopen(fileResource, "r");
	if (!fp) {
		bzero(tempBuffer, BUFSIZE);
		sprintf(tempBuffer, "%s", fileNotFoundResponse);
		send(clientSocket, tempBuffer, strlen(tempBuffer), 0);
		return;
	}
	fread(existingContent, requestedfileSize, 1, fp);
	fclose(fp);
	if (remove(fileResource) == 0) {
		fp = fopen(fileResource, "w");
		char *finalOutput = malloc(
				strlen(existingContent) + strlen(tempBuffer));
		bzero(finalOutput, strlen(existingContent) + strlen(tempBuffer));
		strcat(finalOutput, tempBuffer);
		strcat(finalOutput, existingContent);
		fwrite(finalOutput, 1, strlen(finalOutput), fp);
		fclose(fp);
		free(finalOutput);
	}
	free(existingContent);
}

/*Method to handle all client requests*/
void handleClientRequest(int fd_server, char buf[2048], int fd_client,
		char baseResponse[], extn fileExtensions[]) {
	unsigned long requestedfileSize;
	int fd_img, fileFound = 0, isGetRequest = 0, isPostRequest = 0;
	char *ptr, fileResource[500], requestVersionType[10];
	close(fd_server);
	memset(buf, 0, 2048);
	read(fd_client, buf, 2047);
	printf("%s\n", buf);
	ptr = strstr(buf, " HTTP/1.1");
	if (ptr != NULL)
		sprintf(requestVersionType, " HTTP/1.1");
	else {
		ptr = strstr(buf, " HTTP/1.0");
		sprintf(requestVersionType, " HTTP/1.0");
	}

	if (ptr == NULL) {
		printf("Not an HTTP request");
		bzero(buf, BUFSIZE);
		sprintf(buf, "%s", internalServerErrorResponse);
		send(fd_client, buf, strlen(buf), 0);
	} else {
		*ptr = 0;
		if (((isGetRequest = strncmp(buf, "GET ", 4)) != 0) && ((isPostRequest =
				strncmp(buf, "POST  ", 5)) != 0)) {
			printf("Unsupported HTTP request");
			bzero(buf, BUFSIZE);
			sprintf(buf, "%s", internalServerErrorResponse);
			send(fd_client, buf, strlen(buf), 0);
		} else {

			if (isGetRequest == 0) { //is a get request
				ptr = buf + 4;
			} else if (isPostRequest == 0) { //is a post request
				ptr = buf + 5;
			}

			if (ptr[strlen(ptr) - 1] == '/') {
				strcat(ptr, "index.html");
			}
			strcpy(fileResource, baseResponse);
			strcat(fileResource, ptr);
			printf("%s", fileResource);
			char* requestItemType = strrchr(ptr, '.');
			int i;
			for (i = 0; fileExtensions[i].ext != NULL; i++) {
				if (strcmp(requestItemType + 1, fileExtensions[i].ext) == 0) {
					fileFound = 1;
					if (isGetRequest == 0) {
						fd_img = open(fileResource, O_RDONLY);
					} else if (isPostRequest == 0) {
						while ((strncmp(ptr, "\r\n\r", 3) != 0)) {
							ptr = ptr + 1;
						}
						ptr = ptr + 6;
						writeToFile(ptr, fileResource, fd_client);
						fd_img = open(fileResource, O_RDONLY);
					}
					if (fd_img == -1) {
						bzero(buf, BUFSIZE);
						sprintf(buf, "%s", fileNotFoundResponse);
						send(fd_client, buf, strlen(buf), 0);
						break;
					}
					requestedfileSize = fileSize(fileResource);
					bzero(buf, BUFSIZE);
					sprintf(buf,
							"%s 200 Document Follows\r\n Content-Type: %s \r\n Content-Length: %lu\r\n\r\n",
							requestVersionType, fileExtensions[i].mediatype,
							requestedfileSize);
					send(fd_client, buf, strlen(buf), 0);
					sendfile(fd_client, fd_img, NULL, requestedfileSize);
					close(fd_img);
					break;
				}
			}
			if (fileFound == 0) {
				bzero(buf, BUFSIZE);
				sprintf(buf, "%s", internalServerErrorResponse);
				send(fd_client, buf, strlen(buf), 0);
			}
		}
	}
}

int main(int argc, char **argv) {
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_sock_len = sizeof(client_addr);
	int fd_server, fd_client, flags;
	char buf[2048];
	int opt = 1, portNo, pid;
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portNo = atoi(argv[1]);
	fd_server = checkError(socket(AF_INET, SOCK_STREAM, 0),
			"Could not establish socket for the server", &fd_server);
	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	flags = checkError(fcntl(fd_server, F_GETFL),
			"Could not get the flags on the TCP socket", fd_server);
	checkError(fcntl(fd_server, F_SETFL, flags | O_NONBLOCK),
			"Could not the TCP socket to be non-blocking", fd_server);
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(portNo);
	checkError(
			bind(fd_server, (struct sockaddr *) &server_addr,
					sizeof(server_addr)), "Could not bind server\n",
			&fd_server);
	checkError(listen(fd_server, 10), "Could not listen to new connections\n",
			&fd_server);
	while (1) {
		/* Block till we have an open connection in the queue */
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr,
				&client_sock_len);
		if (fd_client == -1) {
			if (errno == EWOULDBLOCK) {
				/* If there are no pending connections, this will be called. Sleep for a second and check for a new connection */
				sleep(1);
			} else {
				perror("Error accepting connection with client\n");
				exit(1);
			}
		} else {
			printf("Connected to client ... \n");
			pid = fork();
			if (pid < 0) {
				perror("ERROR on fork");
				bzero(buf, BUFSIZE);
				sprintf(buf, "%s", internalServerErrorResponse);
				send(fd_client, buf, strlen(buf), 0);
				exit(1);
			}
			if (pid == 0) {
				handleClientRequest(fd_server, buf, fd_client, baseResponse,
						fileExtensions);
				printf("Closing Connection: no-keep-alive");
				close(fd_client);
				exit(0);
			}
		}
		if (pid > 0) {
			close(fd_client);
			waitpid(0, NULL, WNOHANG);
		}
	}
	close(fd_server);
	return 0;
}

/*Method to check for errors if any*/
int checkError(int id, char * errorMessage, int * socket) {
	if (id == -1) {
		perror(errorMessage);
		close(*socket);
		exit(1);
	}
	return id;
}

/*Method to get size of a file*/
unsigned long fileSize(char *fileName) {
	FILE * f = fopen(fileName, "r");
	fseek(f, 0, SEEK_END);
	unsigned long length = (unsigned long) ftell(f);
	fclose(f);
	return length;
}
