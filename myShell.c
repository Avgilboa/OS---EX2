// gcc myShell.c -o shell
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

void init_shell();
int Dir();
int Copy();
int Unix_command();

int main(int argc , char* argv[]){
    init_shell();
    char command[256];

    while(1){
        fgets(command,256,stdin);
        command[strlen(command)-1] = '\0';
     
        if(strncmp(command,"exit",4) ==0) break;
        else if(strncmp(command,"DIR",3) ==0) Dir();
        else if(strncmp(command,"COPY",4) ==0) Copy(command);
        
        else Unix_command(command);
    }
    printf("goodbye\n");
    return 0;

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
int Unix_command(char *str)
{   
    printf("Unix_command!!\n");
    char *word;
    word = strtok(str,"\t\r\n\v\f ");
    char *newargv[256];
    int i=0;
    while (word && strcmp(word,"|"))
    {
        newargv[i++] = word;
        word = strtok(NULL, "\t\r\n\v\f ");
    }
    newargv[i] = NULL;
    if(word)
    {
        char baff[1028];
        pid_t p1;
        int link[2];
        if (pipe(link) == -1)
        {
            perror("pipe");
        }
        p1=fork();
        if (p1 == 0)
        {
            dup2(link[1],STDOUT_FILENO);
            close(link[0]);
            close(link[1]);
            char bin[] = "/bin/";
            strcat(bin,newargv[0]);
            execve(bin, newargv,NULL);
            perror("execve");
            _exit(0);
        }
        else
        {
            close(link[1]);
            int nbytes = read(link[0],baff,sizeof(baff));
            //wait(NULL);
        }
        char str_new[1028];
        bzero(str_new,1028);
        i=0;
        word = strtok(NULL, " ");
        strcat(str_new,word);
        word = strtok(NULL, " ");
        while (word)
        {
            strcat(str_new,word);
            word = strtok(NULL, " ");
        }
        strcat(str_new," ");
        strcat(str_new,baff);
        word = strtok(baff,"\n");
        /*while (word)
        {
            newargv[i++] = word;
            word = strtok(NULL, "\n");
        }*/
        newargv[i++] = word;
        newargv[i] = NULL;
        Unix_command(str_new);
    }
    else
    {
        for ( i = 0; newargv[i] != NULL; i++)
        {
            printf("ARG[%d] = %s\n",i,newargv[i]);
        }
        
        pid_t p2;
        p2=fork();
        if (p2 == 0)
        {
            char bin[] = "/bin/";
            strcat(bin,newargv[0]);
            execve(bin, newargv,NULL);
            perror("execve");
            _exit(0);
        }
        /*else 
        {
            wait(&p1);
        }*/
    }
    return 1;
    
}