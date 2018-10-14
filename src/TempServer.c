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

char baseResponse[] =
		"HTTP/1.0 200 Ok\r\n"
				"Content-Type: text/html; charset=UTF-8\r\n\r\n"
				"<!DOCTYPE html>\r\n"
				"<html><head><title>Your Server Exists</title>\r\n"
				"<style>body { background-color: blue}</style></head>\r\n"
				"<script src=\"temp.js\"></script>\r\n"
				"<body onload=\"howdy();\"><center><h1>Howdy Buff</h1><img src=\"Kamal.jpg\"/></center></body></html>\r\n";
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
		pid = fork();
		if (pid < 0)
			perror("ERROR on fork");
		if (pid == 0) {
			close(fd_server);
			memset(buf, 0, 2048);
			read(fd_client, buf, 2047);
			printf("%s\n", buf);
			if (!strncmp(buf, "GET /favicon.ico", 16)) {
				fd_img = open("favicon.ico", O_RDONLY);
				requestedfileSize = fileSize("favicon.ico");
				bzero(buf, BUFSIZE);
				sprintf(buf,
						"HTTP/1.1 200 Document Follows\r\n Content-Type: %s\r\n Content-Length: %lu\r\n\r\n",
						"image/ico", requestedfileSize);
				//HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n
				send(fd_client, buf, strlen(buf), 0);
				sendfile(fd_client, fd_img, NULL, requestedfileSize);
				close(fd_img);
			} else if (!strncmp(buf, "GET /Kamal.jpg", 14)) {
				fd_img = open("Kamal.jpg", O_RDONLY);
				requestedfileSize = fileSize("Kamal.jpg");
				bzero(buf, BUFSIZE);
				sprintf(buf,
						"HTTP/1.1 200 Document Follows\r\n Content-Type: %s\r\n Content-Length: %lu\r\n\r\n",
						"image/jpg", requestedfileSize);
				send(fd_client, buf, strlen(buf), 0);
				sendfile(fd_client, fd_img, NULL, requestedfileSize);
				close(fd_img);
			}else if (!strncmp(buf, "GET /temp.js", 12)) {
				fd_img = open("temp.js", O_RDONLY);
				requestedfileSize = fileSize("temp.js");
				bzero(buf, BUFSIZE);
				sprintf(buf,
						"HTTP/1.1 200 Document Follows\r\n Content-Type: %s\r\n Content-Length: %lu\r\n\r\n",
						"application/javascript", requestedfileSize);
				send(fd_client, buf, strlen(buf), 0);
				sendfile(fd_client, fd_img, NULL, requestedfileSize);
				close(fd_img);
			}

			else
				write(fd_client, baseResponse, sizeof(baseResponse));

			close(fd_client);
			printf("Closing connection for client \n");
			exit(0);
		}
		close(fd_client);
	}
	return 0;
}

unsigned long fileSize(char *fileName) {
	FILE * f = fopen(fileName, "r");
	fseek(f, 0, SEEK_END);
	unsigned long length = (unsigned long) ftell(f);
	fclose(f);
	return length;
}
