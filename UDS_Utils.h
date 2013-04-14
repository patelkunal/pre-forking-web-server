#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/socket.h>
#include<sys/types.h>
#include "utils.h"

typedef struct
{
  pid_t pid;
	int status;
	char clientIP[64];
	unsigned int port_no;
} message;

int sockfd[2]; /* sockfd[0] --> read data
 sockfd[1] --> write data */
void createSocketpair(void)
{
	if (socketpair(AF_UNIX, SOCK_STREAM, 0, sockfd) < 0)
	{
		perror("socketpair() failed !!!");
		exit(-1);
	}
}
void sendChildMessage(pid_t child_pid, int status, char * clientIP, unsigned int port_no)
{
	//close(sockfd[0]); // we are not reading anything from parent
	message msg;
	//memset(msg, 0, sizeof(message));
	msg.pid = child_pid;
	msg.status = status;
	strcpy(msg.clientIP, clientIP);
	msg.port_no = port_no;
	if (write(sockfd[1], &msg, sizeof(msg)) < 0)
	{
		perror("error in writing data in UDS socket, exiting");
		exit(-1);
	}
	//close(sockfd[1]);
}
void recvChildMessage(message *child_msg)
{
	memset(child_msg, 0, sizeof(message));
	//close(sockfd[1]); // we are not writing anything to child through socket
	if (read(sockfd[0], child_msg, sizeof(message)) < 0)
	{
		perror("error in reading data from UDS socket, exiting");
		exit(-1);
	}
	//close(sockfd[0]);
}
