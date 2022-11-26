//gcc nc.c -o nc

#include <arpa/inet.h> // inet_addr()
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h> // bzero()
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
#define SA struct sockaddr
void func(int sockfd)
{
	char buff[MAX];
	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		printf("Enter the message : ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n')
			;
		write(sockfd, buff, sizeof(buff));
		bzero(buff, sizeof(buff));
		read(sockfd, buff, sizeof(buff));
		printf(" %s \n", buff);
		// if ((strncmp(buff, "exit", 4)) == 0) {
		// 	printf("Client Exit...\n");
		// 	break;
		// }
	}
}

int main(int argc , char* argv[])
{

	if(argc != 3 ){
		perror("Usage : ./nc [IP] [Port] \n");
		exit(1);
	}

	uint16_t _port = (uint16_t)atoi(argv[2]);
	in_addr_t IP = inet_addr(argv[1]);
	int sockfd, connfd;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
	//	printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = IP;
	servaddr.sin_port = _port;

	// connect the client socket to server socket
	if (connect(sockfd, (SA*)&servaddr, sizeof(servaddr))
		!= 0) {
		printf("connection with the server failed...\n");
		exit(0);
	}
	else
		printf("connected to %s port: %s succeed\n" , argv[1] , argv[2]);

	// function for chat
	func(sockfd);

	// close the socket
	close(sockfd);
}
