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
#define BUFSIZE 2048
unsigned long fileSize(char *);

typedef struct {
	char *ext;
	char *mediatype;
} extn;

extn fileExtensions[] = { { "gif", "image/gif" }, { "txt", "text/plain" }, {
		"jpg", "image/jpg" }, { "js", "application/javascript" }, { "css",
		"text/css" }, { "jpeg", "image/jpeg" }, { "png", "image/png" }, { "ico",
		"image/ico" }, { "zip", "image/zip" }, { "gz", "image/gz" }, { "tar",
		"image/tar" }, { "htm", "text/html" }, { "html", "text/html" }, { "php",
		"text/html" }, { "pdf", "application/pdf" }, { "zip",
		"application/octet-stream" }, { "rar", "application/octet-stream" }, {
		0, 0 } };

char fileNotFoundResponse[] =
		"HTTP/1.0 404 Not Found\r\n"
				"<html><head><title>404 Not Found</head></title>\r\n"
				"<body><p>404 Not Found: The requested resource could not be found!</p></body></html>\r\n";

char baseResponse[] = "./www";
int main(int argc, char **argv) {
	printf("Hello");
	struct sockaddr_in server_addr, client_addr;
	socklen_t client_sock_len = sizeof(client_addr);
	int fd_server, fd_client;
	char buf[2048];
	int opt = 1, portNo, pid;
	int fd_img;
	unsigned long requestedfileSize;
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port>\n", argv[0]);
		exit(1);
	}
	portNo = atoi(argv[1]);
	fd_server = socket(AF_INET, SOCK_STREAM, 0);
	if (fd_server < 0) {
		perror("Could not establish socket for the server");
		exit(1);
	}
	setsockopt(fd_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(int));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(portNo);

	if (bind(fd_server, (struct sockaddr *) &server_addr, sizeof(server_addr))
			== -1) {
		perror("Could not bind server\n");
		close(fd_server);
		exit(1);
	}
	if (listen(fd_server, 10) == -1) {
		perror("Could not listen to new connections\n");
		close(fd_server);
		exit(1);
	}
	while (1) {
		fd_client = accept(fd_server, (struct sockaddr *) &client_addr,
				&client_sock_len);
		if (fd_client == -1) {
			perror("Error accepting connection with client\n");
			continue;
		}
		printf("Connected to client ... \n");
		//pid = fork();
		pid = 0;
		if (pid < 0)
			perror("ERROR on fork");
		if (pid == 0) {
			char *ptr, fileResource[500];
			//close(fd_server);
			memset(buf, 0, 2048);
			read(fd_client, buf, 2047);
			printf("%s\n", buf);
			ptr = strstr(buf, " HTTP/");
			if (ptr == NULL) {
				printf("Not an HTTP request");
			} else {
				*ptr = 0;
				if (strncmp(buf, "GET ", 4) != 0) {
					printf("Not an HTTP GET request");
				} else {
					ptr = buf + 4;
					if (ptr[strlen(ptr) - 1] == '/') {
						strcat(ptr, "index.html");
						strcpy(fileResource, baseResponse);
						strcat(fileResource, ptr);
						fd_img = open(fileResource, O_RDONLY);
						requestedfileSize = fileSize(fileResource);
						bzero(buf, BUFSIZE);
						sprintf(buf,
								"HTTP/1.1 200 Document Follows\r\n Content-Type: %s\r\n Content-Length: %lu\r\n\r\n",
								"text/html", requestedfileSize);
						send(fd_client, buf, strlen(buf), 0);
						sendfile(fd_client, fd_img, NULL, requestedfileSize);
						close(fd_img);
					} else {
						strcpy(fileResource, baseResponse);
						strcat(fileResource, ptr);
						printf("%s", fileResource);
						char* requestItemType = strrchr(ptr, '.');
						int i;
						for (i = 0; fileExtensions[i].ext != NULL; i++) {
							if (strcmp(requestItemType + 1,
									fileExtensions[i].ext) == 0) {
								fd_img = open(fileResource, O_RDONLY);
								if (fd_img == -1) {
									bzero(buf, BUFSIZE);
									sprintf(buf,"%s",fileNotFoundResponse);
									send(fd_client, buf, strlen(buf), 0);
									break;
								}
								requestedfileSize = fileSize(fileResource);
								bzero(buf, BUFSIZE);
								sprintf(buf,
										"HTTP/1.1 200 Document Follows\r\n Content-Type: %s\r\n Content-Length: %lu\r\n\r\n",
										fileExtensions[i].mediatype,
										requestedfileSize);
								send(fd_client, buf, strlen(buf), 0);
								sendfile(fd_client, fd_img, NULL,
										requestedfileSize);
								close(fd_img);
								break;
							}
						}
					}
				}
			}
		}
		close(fd_client);
		printf("Closing connection for client \n");
		//exit(0);
	}
	//close(fd_client);
}
//return 0;
//}

unsigned long fileSize(char *fileName) {
	FILE * f = fopen(fileName, "r");
	fseek(f, 0, SEEK_END);
	unsigned long length = (unsigned long) ftell(f);
	fclose(f);
	return length;
}
