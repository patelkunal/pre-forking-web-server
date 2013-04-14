#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include<sys/socket.h>
#include<sys/wait.h>
#include<netdb.h>

#define die(msg) \
  { \
		perror(msg); \
		exit(EXIT_FAILURE);\
	}

#define TRUE 1
#define FALSE !TRUE

#define TABLE_SIZE 512
