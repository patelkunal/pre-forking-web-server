#include <unistd.h> /* fork, close */
#include <stdlib.h> /* exit */
#include <string.h> /* strlen */
#include <stdio.h> /* perror, fdopen, fgets */
#include <sys/socket.h>
#include <sys/wait.h> /* waitpid */
#include <netdb.h>
#include "utils.h"

int listenfd;
typedef struct
{
	int acceptfd;
	struct sockaddr_in clientAddrs;
} clientInfo;
int port = 4242;
//struct sockaddr_in serverAddrs, clientAddrs;

void createSocket()
{
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd == -1)
		die("Error while creating Socket, exiting\n");
	printf("Socket successfully created !!\n");
}
void bindSocket(int port)
{
	struct sockaddr_in serverAddrs;
	memset(&serverAddrs, 0, sizeof(serverAddrs));
	serverAddrs.sin_family = AF_INET;
	serverAddrs.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddrs.sin_port = htons(port);

	if (bind(listenfd, (struct sockaddr *) &serverAddrs, sizeof(serverAddrs)) < 0)
		die("bind() failed, exiting\n");
	printf("bind() successful !!\n");
}
void createServer(int port)
{
	createSocket();
	bindSocket(port);
	if (listen(listenfd, 10) < 0)
		die("listen() failed, exiting");
}
void acceptConnection(clientInfo * c)
{
	int acceptfd;
	unsigned int clientLength;
	clientLength = sizeof(c->clientAddrs);

	acceptfd = accept(listenfd, (struct sockaddr *) &(c->clientAddrs), &clientLength);
	if (acceptfd == -1)
		die("accept() failed, exiting");
	//printf("child (%d) is handling Client @ %s on port_number = %d\n", getpid(), (char *) inet_ntoa(clientAddrs.sin_addr.s_addr), ntohs(clientAddrs.sin_port));
	c->acceptfd = acceptfd;
}

int recvMsg(int fd, char buffer[], int size)
{
	int recvMsgSize;
	if ((recvMsgSize = recv(fd, buffer, size, 0)) < 0)
		die("recv() failed, exiting");
	return recvMsgSize;
}

int sendMsg(int fd, char buffer[], int size)
{
	int sentMsgSize;
	sentMsgSize = send(fd, buffer, size, 0);
	if (sentMsgSize <= 0)
		die("send() failed, exiting");
	return sentMsgSize;
}

void handleClient(int acceptfd)
{
	char echoBuffer[10240];
	int recvMsgSize, sentMsgSize;

	recvMsgSize = recvMsg(acceptfd, echoBuffer, 10240);
	//sleep(3);
	char *reply = "HTTP/1.1 200 OK\n";
	sentMsgSize = sendMsg(acceptfd, reply, strlen(reply));
	close(acceptfd);
}

/*
 How to use TCP_Utils.h --- Sample program..

 int main()
 {
 int acceptfd;
 createServer();
 char * client_ip;

 printf("Waiting for connection !!\n");
 while(1)
 {
 acceptfd = acceptConnection();
 client_ip = (char *)inet_ntoa(clientAddrs.sin_addr.s_addr);
 printf("Handling TCP Client %s\n", client_ip);
 if(fork() == 0)
 {
 close(listenfd);
 handleClient(acceptfd);
 close(acceptfd);
 exit(0);
 }
 close(acceptfd);
 }
 close(listenfd);
 return 0;
 }

 */
