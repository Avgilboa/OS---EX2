// gcc ncL.c -o nc-L  

#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
//#define PORT 8080
#define SA struct sockaddr

// Function designed for chat between client and server.
void unix_command(char* str){
	printf("This process the unix command: %s" , str);
}

void func(int connfd)
{
	char buff[MAX];
	int n;
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);
		int fd[2];
		if(pipe(fd) < 0){
			perror("pipe");
			exit(2);
		}
		int pid1 = fork();
		if(pid1 == 0){
			dup2(fd[1], STDOUT_FILENO );
			close(fd[1]);
			close(fd[0]);
			if (read(connfd, buff, sizeof(buff)) >0){
				printf("%s" , buff);  /// To client : ", buff);
				bzero(buff, MAX);
			}
			// print buffer which contains the client contents

		} 
		else
		{
			dup2(fd[0], STDIN_FILENO);
			close(fd[1]);
			close(fd[0]);
			bzero(buff, MAX);
			n = 0;
			// copy server message in the buffer
			while ((buff[n++] = getchar()) != '\n');
			int res[2];
			if(pipe(res) <0 ){perror("pipe");}

			dup2(res[1], STDOUT_FILENO);
			unix_command(buff);
			read(res[0], buff, sizeof(buff));
			// and send that buffer to client
			write(connfd, buff, sizeof(buff));
		}
		

	}
}

// Driver function
int main(int argc , char* argv[])
{
	if( argc !=2){
		perror("Usage : ./ncL [port] \n");
		exit(1);
	}
	uint16_t _port = (uint16_t)atoi(argv[1]);
	in_addr_t IP = htonl(INADDR_ANY);

	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	//else
	//	printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = IP ;//htonl(INADDR_ANY);
	servaddr.sin_port = _port;

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	//else
	//	printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("listening..\n");
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("listen to [any] in port %s \n" , argv[1]);

	// Function for chatting between client and server
	func(connfd);

	// After chatting close the socket
	close(sockfd);
}