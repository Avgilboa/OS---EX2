// gcc myShell.c -o Pshell
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <libgen.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include<limits.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h> // read(), write(), close()
#define MAX 80
//#define PORT 8080
#define SA struct sockaddr

int init_shell();
int Dir();
int Copy();
int Unix_command();
int pip();
int redirect();
int direct();
int client(char* command , char* ip , char* port); // }
void funclient();
int server(char* port) ; //{ 
void funcserv();

int main(int argc , char* argv[]){
    if(strcmp(argv[1], "s") == 0){
        server("5555");
    }
    else if(strcmp(argv[1], "c") == 0){
        client("ls -l", "127.0.0.1", "5555");
    }
    //init_shell(NULL);
    printf("goodbye\n");
    return 0;
}
int init_shell(char *str)
{
    char command[256];
    bzero(command,256);
    if (!str)
    {
        fgets(command,256,stdin);
        command[strlen(command)-1];
    }
    else strcpy(command,str);
    if (strchr(command, '>')){
        char* word;
        char left[256] = {'\0'};
        char right[256] = {'\0'};
        word = strtok(command,">");  // ls -l
        strcpy(left, word);
        word = strtok(NULL,"\n");
        strcpy(right, &(word[1]));
        printf("left is: %s\n right is: %s\n",left,right);
        redirect(left, right);
    }

    else if (strchr(command, '<')){
        char* word;
        char left[256] = {'\0'};
        char right[256] = {'\0'};
        word = strtok(command,"<");  // ls -l
        strcpy(left, word);
        word = strtok(NULL,"\n");
        strcpy(right, &(word[1]));
        direct(left, right);
    }

    else if( strchr(command , '|') ){
        char* word;
        char left[256] = {'\0'};
        char right[256] = {'\0'};
        word = strtok(command," | ");  // ls -l

        strcpy(left, word);
        word = strtok(NULL,"\n");

        strcpy(right, &(word[2]));
        //printf("the second command is : %s \n" , right);
        pip(left, right);
        return 0;

    }
    //if(strncmp(command,"exit",4) ==0) break;

    else if(strncmp(command,"DIR",3) ==0) Dir();
    else if(strncmp(command,"COPY",4) ==0) Copy(command);
    
    else
    { 
        char* word;
        word = strtok(command,"\n");
        Unix_command(word);
    }
    bzero(command, 256);
    wait(NULL);
    return 0;
}
int redirect(char* str , char* filename ){
    int fd[2];
    pipe(fd);
    dup2(fd[1], STDOUT_FILENO);
    init_shell(str);
    char buff[1024] = {'\0'};
    read(fd[0], buff, 1024);
    FILE *file = fopen(filename, "w");

    fprintf(file,"%s",buff);
    fclose(file);
    return 1;
}
int direct(char* str , char* filename ){
    int fd[2];
    pipe(fd);
    char source[1024] = {'\0'};
    int f1;
    if ((f1 = open(filename ,O_RDONLY)) < 0) //open file 1 for read
    {
        printf("%s\n",filename);
        perror(" cant open sorce file");
        exit(1);
    }
    dup2(f1, STDIN_FILENO);
    close(fd[1]);
    close(fd[0]);
    init_shell(str);
    

}
int Dir(){
    DIR * dir;
    if( (dir =opendir(".")) == NULL){
        perror("cannot open");
        exit(0);
        
    }
    struct dirent * dp;
    while(( dp = readdir(dir))!= NULL){
        if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
            continue;
        printf("%s \n",dp->d_name);
    }
    closedir(dir);
    return 1;
}
int Copy(char* str)
{
    char src[128], dest[128];
    if (str[4]!='<')
    {
        perror("Usage: COPY<src><dest>");
        exit(1);
    }
    int i =5;
    int index = 0;
    
    while (1)
    {
        src[index++] = str[i++];
        if (str[i] == '>')
        {
            src[index] = '\0';
            break;
        } 
    }
    if (str[++i]!='<')
    {
        perror("Usage: COPY<src><dest>");
        exit(1);
    }
    i++;
    index=0;
    while (1)
    {
        dest[index++] = str[i++];
        if (str[i] == '>')
        {
            dest[index] = '\0';
            break;
        } 
    }
    //from here its like the previus Ex, copy between two file. 
    int fdsrc;
    int fddst;
    if ((fdsrc = open(src ,O_RDONLY)) < 0) //open file 1 for read
    {
        printf("%s\n",src);
        perror(" cant open sorce file");
        exit(1);
    }
    if ((fddst = open(dest ,O_RDONLY | O_WRONLY | O_TRUNC | O_WRONLY|O_CREAT , 0666)) < 0) //open file 2 to write
    {
        perror(" cant open dest file");
        exit(1);
    }
    ssize_t readBytes, wroteBytes;
    char *buf[10];
    lseek(fddst,0,SEEK_SET); //start the writing from the begining of the file
    do
    {
        readBytes = read(fdsrc, buf, 10); //read every time 10 byte from file 1
        if (readBytes < 0)
        {
            printf("I/O errors\n");
            exit(-10);
        }

        wroteBytes = write(fddst, buf, readBytes); // write the bytes we read to file 2
        if (wroteBytes < 0)
        {
            printf("this is the error\n");
            if (errno == EDQUOT)
            {
                printf("ERROR: out of quota.\n");
            }
            else if (errno == ENOSPC)
            {
                printf("ERROR: not enough disk space.\n");
            }
        }
    } while (readBytes > 0);
    close(fdsrc);
    close(fddst);
    return 1;
}
int pip(char* lft , char* right){
    int fd[2], pid1, pid2;
    if(pipe(fd) <0){
        perror("pipe");
        exit(2);
    }

    if(fork() == 0){
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        if(strcmp(lft, "DIR") == 0){
            Dir();
        }
        else{
            Unix_command(lft);
        }
        close(fd[1]);
    }

        else{

            close(fd[1]);
            char buff[256] ={'\0'};
            // read(fd[0], buff , 256);
            // printf("%s", buff);
            dup2(fd[0], STDIN_FILENO );

            // read(fd[0], buff , 256);
            // printf("%s\n", buff);
            close(fd[0]);

            if(strcmp(lft, "DIR") == 0){
            Dir();
            }
            else{
                Unix_command(right);
            }
            
            
        }
        return 1;
        
    }

int Unix_command(char *str)
{   
    //printf("str is : %s",str);
    char *word;
    word = strtok(str," ");
    char *newargv[256];
    int i=0 , pid1;
    while (word)
    {
        newargv[i++] = word;
        word = strtok(NULL, " ");
    }
    newargv[i] = NULL;
    if((pid1 = fork()) < 0){
        perror("fork");
        exit(2);
    }
    if(pid1 ==0 ){
        
        char bin[] = "/bin/";
        strcat(bin,newargv[0]);
        execve(bin, newargv ,NULL); // run the first program
        perror("execve"); //only if have error
        _exit(2);
    }
    else 
    {
        wait(NULL);
    }
    return 1;
    
}



void funcserv(int connfd)
{
	char buff[MAX];
	int n;
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);
		int pid1 = fork();
		if(pid1 == 0){
			if (read(connfd, buff, sizeof(buff)) >0){
				printf("%s" , buff);  ///t To client : ", buff);
				bzero(buff, MAX);
			}
			// print buffer which contains the client contents

		} 
		else
		{
			bzero(buff, MAX);
			n = 0;
			// copy server message in the buffer
			while ((buff[n++] = getchar()) != '\n');
			// and send that buffer to client
			write(connfd, buff, sizeof(buff));
		}
		

	}
}



int server(char* port){
    uint16_t _port = (uint16_t)atoi(port);
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





void funclient(int sockfd , char* command){
	char buff[MAX];
	int n;
	for (;;) {
		bzero(buff, sizeof(buff));
		
		int pid1 = fork();
		if(pid1 == 0){
			bzero(buff, sizeof(buff));
			if((read(sockfd, buff, sizeof(buff)))>0){
				printf("%s", buff);
				bzero(buff, sizeof(buff));
			}

		}
		else{
			n = 0;
			while ((buff[n++] = getchar()) != '\n');
			write(sockfd, buff, sizeof(buff));
		}
	
		
	}
}


int client(char* command, char* ip , char* port){
	uint16_t _port = (uint16_t)atoi(port);
	in_addr_t IP = inet_addr(ip);
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
		printf("connected to %s port: %s succeed\n" , ip , port);

	// function for chat
	funclient(sockfd);

	// close the socket
	close(sockfd);
}



		

