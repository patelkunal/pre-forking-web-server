#include "TCP_Utils.h"
#include "UDS_Utils.h"
#include<signal.h>
//#include "utils.h"

typedef struct
{
  pid_t pid;
	int status;
	int requestsHandled;
} child_info;

child_info children[TABLE_SIZE];

int StartServer, MinSpareServer, MaxSpareServer, MaxClient, MaxRequestPerChild;
unsigned int totalChildren, currentRequests, spareChildren;
int acceptfd;

// Function prototypes
void childFunction(void);
void addChildInformation(pid_t, int, int);
int getHandledRequests(pid_t);
int getChildStatus(pid_t);
void deleteChildInformation(pid_t);
void updateChildInformation(pid_t, int);
pid_t getChildPIDtoDelete(void);
void printServerVariables(void);
void printChildrenInformation(void);

//signal handlers
void sigClearSocket(int);
void sigPrintChildrenInfo(int);
void sigIgnore(int);

int main(int argc, char *argv[])
{

	// REMOVE THIS COMMENT TO TAKE COMMAND LINE ARGS
	if (argc != 6)
	{
		printf("usage: %s <StartServer> <MinSpareServer> <MaxSpareServer> <MaxClient> <MaxRequestPerClient>\n", argv[0]);
		exit(-1);
	}

	StartServer = atoi(argv[1]);
	MinSpareServer = atoi(argv[2]);
	MaxSpareServer = atoi(argv[3]);
	MaxClient = atoi(argv[4]);
	MaxRequestPerChild = atoi(argv[5]);

	signal(SIGINT, sigPrintChildrenInfo);

	// Initialise table
	memset(children, -1, sizeof(children));

	createServer(port);
	createSocketpair();
	int i;
	pid_t cpid;
	printf("Parent pid = %d\n", getpid());
	for (i = 0; i < StartServer; i++)
	{
		if ((cpid = fork()) == -1)
			die("fork() failed, exiting");
		if (cpid == 0) // child
		{
			childFunction();
			exit(0);
		}
		addChildInformation(cpid, 0, 0);
		//printf("child pid = %d, child status = %d and requests = %d\n", cpid, getChildStatus(cpid), getHandledRequests(cpid));
	}

	printChildrenInformation();

	message msg;
	pid_t cp;
	totalChildren = StartServer;
	currentRequests = 0;

	spareChildren = totalChildren - currentRequests;
	printf("Initial Server Configuration !!\n");
	printf("MinSpareServer = %d\tMaxSpareServer = %d\ttotalChildren = %d\tcurrentRequests = %d\tspareChildren = %d !!\n", MinSpareServer, MaxSpareServer,
			totalChildren, currentRequests, spareChildren);
	printf("\nServer log !!! \n");
	int status;
	while (1) // Parent is now waiting for child's reply and self regulated logic
	{
		// Self regulated logic

		printf("\n!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		spareChildren = totalChildren - currentRequests;
		printf("\n");

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		printChildrenInformation();
		printf("\n");
		memset(&msg, 0, sizeof(msg));
		//recvChildMessage(&msg);
		read(sockfd[0], &msg, sizeof(message));

		if (msg.status == 1)
			printf("child (%d) is handling Client @ %s on port_number = %d\n", msg.pid, msg.clientIP, msg.port_no);
		else
			printf("Child (%d) has finished client request with ReqsHandled = %d!!\n", msg.pid, getHandledRequests(msg.pid));

		// Store this child struct in table..
		updateChildInformation(msg.pid, msg.status);
		//printChildrenInformation();
		if (getChildStatus(msg.pid) != 1) // Free child
		{
			if (getHandledRequests(msg.pid) >= MaxRequestPerChild)
			{
				printf("MaxRequestsPerChild --> Child to be killed = %d\n", msg.pid);
				//printf("Child %d is killed !!\n", p);

				deleteChildInformation(msg.pid);
				kill(msg.pid, SIGKILL);
				waitpid(msg.pid, &status, 0);
				//printf("(Killed) Child pid = %d and status = %d\n", msg.pid, status);
				cp = fork();
				if (cp == -1)
					die("fork() error, exiting");
				if (cp == 0) // child
				{
					childFunction();
					exit(0);
				}
				else // parent
				{
					addChildInformation(cp, 0, 0);
					printf("Recycling --> Added New Child --> child pid = %d\n\n", cp);
				}
			}
		}

		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

		if (spareChildren < MinSpareServer) // fork child and call child function
		{
			//printf("totalChildren = %d and MaxClient = %d\n", totalChildren, MaxClient);
			if (totalChildren > MaxClient)
			{
				printf("MaxClient limit exceeded, can't fork new child, wait for a while\n");
			}
			else
			{
				totalChildren++;
				spareChildren++;
				//printf("Forking a new child !!\n");
				pid_t cp = fork();
				if (cp == -1)
					die("fork() error, exiting");
				if (cp == 0) // child
				{
					childFunction();
					exit(0);
				}
				else
				{ // parent
					addChildInformation(cp, 0, 0);
					printf("Too few spare servers --> New child forked --> ");
					//printf("child pid = %d, child status = %d and requests = %d\n", cp, getChildStatus(cp), getHandledRequests(cp));
					printf("child pid = %d\n", cp);
				}
				//printChildrenInformation();
			}
		}
		else if (spareChildren > MaxSpareServer) // kill a child
		{
			pid_t p = getChildPIDtoDelete();

			if (p > 0)
			{
				printf("Too many children --> child to be killed = %d\n", p);
				//printf("Child %d is killed !!\n", p);
				totalChildren--;
				spareChildren--;
				deleteChildInformation(p);
				kill(p, SIGKILL);
				waitpid(p, &status, 0);
				//printf("(Killed) Child pid = %d and status = %d\n", p, status);
			}
			//printChildrenInformation();
		}
		printServerVariables();
	}

	while (waitpid(-1, NULL, 0) > 0)
		; // For zombie process
	return 0;
}

void childFunction()
{
	//signal(SIGTERM, sigClearSocket);
	//signal(SIGUSR1, sigIgnore);
	signal(SIGINT, SIG_IGN);
	struct sockaddr_in clientAddrsTemp;
	clientInfo c;
	char clientIP[64];
	unsigned int port_no;
	while (TRUE)
	{
		acceptConnection(&c);

		clientAddrsTemp = c.clientAddrs;
		strcpy(clientIP, (char *) inet_ntoa(clientAddrsTemp.sin_addr.s_addr));
		port_no = ntohs(clientAddrsTemp.sin_port);

		sendChildMessage(getpid(), 1, clientIP, port_no); // Send UDS Message to parent --> BUSY (1)
		handleClient(c.acceptfd);
		sendChildMessage(getpid(), 0, clientIP, port_no); // Send UDS Message to parent --> FREE (0)
		close(acceptfd);
		sleep(2);
		//pause();
	}
	close(listenfd);
}
void sigClearSocket(int signo)
{
	printf("Child (%d) has received SIGTERM !!\n", getpid());

	close(listenfd);
	shutdown(acceptfd, SHUT_RDWR);

	exit(0);
}

void sigPrintChildrenInfo(int signo)
{
	signal(SIGINT, sigPrintChildrenInfo);
	printf("\n************** Server Status **********************\n");
	int i;
	int activeChildren = 0;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		if (children[i].pid == -1)
			continue;
		else
		{
			printf("child pid = %d, child status = %d and requests = %d\n", children[i].pid, children[i].status, children[i].requestsHandled);
			if (children[i].status == 1)
			{
				activeChildren++;
			}
		}
	}
	printf("Total Children = %d and active children = %d\n", totalChildren, activeChildren);
	printf("*****************************************************\n");
}

void addChildInformation(pid_t pid, int status, int requestHandler)
{
	int i = 0;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		if (children[i].pid == -1)
		{
			children[i].pid = pid;
			children[i].status = status;
			children[i].requestsHandled = requestHandler;
			break;
		}
		else
			continue;
	}
}

pid_t getChildPIDtoDelete()
{
	int i;
	pid_t cpid;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		if (children[i].pid != -1) // value should not be -1
		{
			if (children[i].status != 1) // any client should not served by that client
				cpid = children[i].pid;
			break;
		}
	}
	return cpid;
}

int getHandledRequests(pid_t pid)
{
	int req = -99;
	int i;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		if (children[i].pid == -1)
			continue;
		else if (children[i].pid == pid)
		{
			req = children[i].requestsHandled;
			break;
		}
	}
	return req;
}

int getChildStatus(pid_t pid)
{
	int status = -1;
	int i;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		if (children[i].pid == -1)
			continue;
		else if (children[i].pid == pid)
		{
			status = children[i].status;
			break;
		}
	}
	return status;
}
void deleteChildInformation(pid_t pid)
{
	int i;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		if (children[i].pid == -1)
			continue;
		else if (children[i].pid == pid)
		{
			children[i].pid = -1;
			children[i].status = -1;
			children[i].requestsHandled = -1;
			break;
		}
	}
}
void updateChildInformation(pid_t pid, int status)
{
	int i;
	for (i = 0; i < TABLE_SIZE; i++)
	{
		if (children[i].pid == -1)
		{
			continue;
		}
		if (children[i].pid == pid)
		{
			children[i].status = status;
			if (status == 1)
			{
				children[i].requestsHandled++;
				currentRequests++;
				spareChildren--;
			}
			else
			{
				currentRequests--;
				spareChildren++;
			}
			//printf("child pid = %d, child status = %d and requests = %d\n", children[i].pid, children[i].status, children[i].requestsHandled);
			break;
		}
	}
}
void printChildrenInformation()
{
	int i;
	printf("\n*********************Children Information*************************\n");
	for (i = 0; i < TABLE_SIZE; i++)
	{
		if (children[i].pid == -1)
			continue;
		else
		{
			printf("child pid = %d, child status = %d and requests = %d\n", children[i].pid, children[i].status, children[i].requestsHandled);
		}
	}
	printf("********************************************************************\n");

}
void printServerVariables(void)
{
	//printf("MinSpareServer = %d\tMaxSpareServer = %d\ttotalChildren = %d\tcurrentRequests = %d\tspareChildren = %d !!\n", MinSpareServer, MaxSpareServer,	totalChildren, currentRequests, spareChildren);
	printf("totalChildren = %d\tcurrentRequests = %d\tspareChildren = %d !!\n", totalChildren, currentRequests, spareChildren);
}
void sigIgnore(int signum)
{
	//printf("Inside signal handler --> pid = %d !!\n", getpid());
}
