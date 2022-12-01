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

int init_shell();
int Dir();
int Copy();
int Unix_command();
int pip();
int redirect();
int direct();

int main(int argc , char* argv[]){
    
    init_shell(NULL);
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

    else if(strchr(command,'{')) //only port
    {
        char *word;
        word = strtok(command,"{");
        //server(word[1]);
    }
    else if (strchr(command,'}'))
    {
        char *word;
        char comm[256], port[256], ip[256];
        word = strtok(command,"}");
        strcpy(comm,word);
        word = strtok(NULL,":");
        strcpy(ip,&word[1]);
        word = strtok(NULL," ");
        strcpy(port,word);
        printf("command is:%s\nip is:%s\nport is:%s",comm,ip,port);
        //client(comm,ip,port);

    }

    else if( strchr(command , '|') ){
        char* word;
        char left[256] = {'\0'};
        char right[256] = {'\0'};
        word = strtok(command," | ");  // ls -l

        strcpy(left, word);
        word = strtok(NULL,"\n");
        printf("the command is: %s\n",word);
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
