// gcc pipeShell.c -o Pshell
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

void init_shell();
int Dir();
int Copy();
int Unix_command();
int pip();
int redirect();
int direct();

int main(int argc , char* argv[]){
    init_shell();
    char command[256];
     //// > < | { }   Dir | wc > file1.txt
    //while()
        fgets(command,256,stdin);
        command[strlen(command)-1];

        if (strchr(command, '>')){
            char* word;
            char left[256] = {'\0'};
            char right[256] = {'\0'};
            word = strtok(command," > ");  // ls -l
            strcpy(left, word);
            word = strtok(NULL,"\n");
            strcpy(right, &(word[2]));
            redirect(left, right);
        }

        else if (strchr(command, '<')){
            char* word;
            char left[256] = {'\0'};
            char right[256] = {'\0'};
            word = strtok(command," < ");  // ls -l
            strcpy(left, word);
            word = strtok(NULL,"\n");
            strcpy(right, &(word[2]));
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

    // FILE *f1 = fopen("out.txt", "w");
    // fclose(f1);
    // char *newargv2[] = {"rm","out.txt",NULL};
    // execve("/bin/rm", newargv2,NULL);
    return 0;

}
int redirect(char* str , char* filename ){
    int fd[2];
    pipe(fd);
    dup2(fd[1], STDOUT_FILENO);
    Unix_command(str);
    char buff[1024] = {'\0'};
    read(fd[0], buff, 1024);
    FILE *file = fopen(filename, "w");

    int results = fputs(buff, file);
    fclose(file);
    return 1;
}
int direct(char* str , char* filename ){
    int fd[2];
    pipe(fd);
    char source[1024] = {'\0'};
    FILE *fp = fopen(filename, "r");
    char ch;
    if(fp != NULL)
    {
        while( (ch = (getc(fp)) ) != EOF)
        {
            strcat(source, &ch);
        }
        fclose(fp);
        write(fd[1], source , 1024);
        dup2(fd[0], STDIN_FILENO);
        close(fd[1]);
        close(fd[0]);
        Unix_command(str);
    }

}


void init_shell(){
    printf("------------------------\n  my_shell \n------------------------ \n");
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
    char *word;
    char *newargv[256];
    int i=0, pid1;

    word = strtok(str," ");
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
    else{
        wait(NULL);
        return 1;
    }
    
    return 0;
    
    
}